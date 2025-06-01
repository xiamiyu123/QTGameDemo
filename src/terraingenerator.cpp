#include "terraingenerator.h"
#include <QDateTime>
#include <QPen>
#include <QBrush>
#include <QtMath>
#include <QRandomGenerator>
#include "debuglogger.h"
#include <QMutexLocker>
#include "physical.h"
#include "npcentity.h"

const qreal MIN_ROCK_DISTANCE = 100; // 最小石头间距
const qreal MAX_SLOPE_FOR_ROCK = 0.4; // 允许生成石头的最大斜率（绝对值）

TerrainGenerator::TerrainGenerator(QGraphicsScene *scene, QObject *parent)
    : QObject(parent),
      m_scene(scene) {
    // 初始化随机种子
    m_seed = QDateTime::currentMSecsSinceEpoch();
    m_randomGenerator = QRandomGenerator(m_seed); // 创建自己的随机生成器实例
    m_perlin = PerlinNoise(m_seed); // 使用随机种子初始化Perlin噪声类
    
    // 创建线程对象
    m_generatorThread = new TerrainGeneratorThread(this, this);
    
    // 连接信号和槽
    connect(m_generatorThread, &TerrainGeneratorThread::chunkGenerated,
            this, &TerrainGenerator::addChunkToScene);
    
    // 启动线程
    m_generatorThread->start();
}

void TerrainGenerator::initialize() {
    // 生成初始地形块
    for (int i = -1; i <= 1; ++i) {
        generateChunk(i);
    }
}

void TerrainGenerator::updateTerrain(qreal playerX) {
    // 计算玩家当前所在的地形块
    int currentChunk = floor(playerX / CHUNK_WIDTH);

    // 请求生成前方的地形块
    for (int i = currentChunk - 1; i <= currentChunk + VIEW_CHUNKS; ++i) {
        // 检查是否已经存在或已经请求生成
        QMutexLocker locker(&m_mutex);
        if (!m_chunks.contains(i) && !m_generatedPaths.contains(i)) {
            // 请求在线程中生成
            m_generatorThread->requestChunkGeneration(i);
        }
    }

    // 移除远离玩家的地形块
    removeDistantChunks(currentChunk);
}

qreal TerrainGenerator::getTerrainHeight(qreal x) const {
    // 计算点所在的地形块
    int chunkIndex = floor(x / CHUNK_WIDTH);
    qreal localX = x - chunkIndex * CHUNK_WIDTH;

    // 如果地形块不存在，返回默认高度
    if (!m_chunkPoints.contains(chunkIndex)) {
        return 0;
    }

    const QVector<QPointF> &points = m_chunkPoints[chunkIndex];

    // 找到x坐标最接近的两个点
    int i = 0;
    while (i < points.size() && points[i].x() < localX) {
        i++;
    }

    if (i == 0) {
        return points[0].y();
    }

    if (i >= points.size()) {
        return points.last().y();
    }

    // 线性插值
    qreal x1 = points[i - 1].x();
    qreal y1 = points[i - 1].y();
    qreal x2 = points[i].x();
    qreal y2 = points[i].y();

    // 计算斜率和高度
    qreal slope = (y2 - y1) / (x2 - x1);
    return y1 + slope * (localX - x1);
}

void TerrainGenerator::generateChunk(int chunkIndex) {
    if (m_chunks.contains(chunkIndex)) {
        return;
    }

    // 定义地形参数
    const int POINTS = 2000; // 每个地形块上的点数量
    const int BASE_HEIGHT = 300; // 地基高度
    const int HEIGHT_VARIATION = 10; // 高度变化范围
    const int BASE_SLOPE_FACTOR = 1400; // 基本下降趋势因子
    const int TRANSITION_ZONE = 400; // 两侧过渡区域的点数

    // 创建地形点
    QVector<QPointF> points;

    // 确保与前一个块平滑连接
    qreal startHeight = BASE_HEIGHT;
    qreal startSlope = 0.0; // 记录起始斜率
    qreal SLOPE_FACTOR = BASE_SLOPE_FACTOR;

    // //随机变化下降趋势因子
    // //因为会引起我不会修的bug暂时停用
    // int randomFactor = m_randomGenerator.bounded(0, 2);
    // if (randomFactor == 0) {
    //     SLOPE_FACTOR -= m_randomGenerator.bounded(0, 200);
    // } else if (randomFactor == 1) {
    //     SLOPE_FACTOR += m_randomGenerator.bounded(0, 100);
    // }

    if (chunkIndex > 0 && m_chunkPoints.contains(chunkIndex - 1)) {
        const QVector<QPointF> &prevPoints = m_chunkPoints[chunkIndex - 1];
        startHeight = prevPoints.last().y();

        // 计算前一块末尾的斜率
        if (prevPoints.size() >= 2) {
            qreal lastDelta = prevPoints.last().y() - prevPoints[prevPoints.size() - 2].y();
            qreal lastDx = prevPoints.last().x() - prevPoints[prevPoints.size() - 2].x();
            startSlope = lastDelta / lastDx;
        }
    } else if (chunkIndex < 0 && m_chunkPoints.contains(chunkIndex + 1)) {
        const QVector<QPointF> &nextPoints = m_chunkPoints[chunkIndex + 1];
        startHeight = nextPoints.first().y();

        // 计算后一块开始的斜率
        if (nextPoints.size() >= 2) {
            qreal firstDelta = nextPoints[1].y() - nextPoints[0].y();
            qreal firstDx = nextPoints[1].x() - nextPoints[0].x();
            startSlope = firstDelta / firstDx;
        }
    }

    // 第一个点
    points.append(QPointF(0, startHeight));


    // 生成随机地形点
    for (int i = 1; i < POINTS; ++i) {
        qreal x = (qreal) i / POINTS * CHUNK_WIDTH;
        qreal globalX = x + chunkIndex * CHUNK_WIDTH;

        // 使用多层柏林噪声函数生成地形
        qreal noiseValue = noise(globalX * 1); // 因为一层就够好所以暂时只用一层

        // 平滑的下降趋势
        qreal globalFactor = chunkIndex * BASE_SLOPE_FACTOR; // 基于位置的全局下降因子
        qreal localFactor = qSqrt((qreal) i / POINTS) * SLOPE_FACTOR; // 块内局部下降
        qreal downwardTrend = globalFactor + localFactor;

        // 计算基础高度
        qreal baseHeight = BASE_HEIGHT + downwardTrend;
        // 添加随机变化
        // 在过渡区域内，使用权重混合来平滑过渡
        qreal blendFactor = 1.0;
        // 在过渡区域内，使用平滑函数（而非线性）混合来过渡
        if (i < TRANSITION_ZONE) {
            // 使用余弦插值函数替代线性插值，提供更自然的过渡
            qreal t = (qreal) i / TRANSITION_ZONE;
            qreal smoothT = (1 - qCos(t * M_PI)) * 0.5; // 余弦平滑函数

            // 计算预期高度（根据起始点和斜率）
            qreal expectedHeight = startHeight + startSlope * x;

            // 添加小幅随机变化，但保持基本趋势
            qreal transitionNoise = noise(globalX * 0.02 + 100) * HEIGHT_VARIATION * 0.3;
            expectedHeight += transitionNoise * smoothT; // 逐渐引入噪声

            // 混合预期高度和基础高度
            baseHeight = expectedHeight * (1 - smoothT) + baseHeight * smoothT;

            // 噪声强度随着远离连接点而增加
            noiseValue *= smoothT;
        }

        // 最终高度计算
        qreal height = baseHeight + noiseValue * HEIGHT_VARIATION * blendFactor;
        points.append(QPointF(x, height));
    }


    // 存储点以便后续插值
    m_chunkPoints[chunkIndex] = points;

    // 创建地形路径
    QPainterPath path;
    path.moveTo(points.first());

    // 添加所有点
    for (int i = 1; i < points.size(); ++i) {
        path.lineTo(points[i]);
    }

    // 完成地形封闭
    path.lineTo(CHUNK_WIDTH, 5000000);
    path.lineTo(0, 5000000);
    path.closeSubpath();

    // 创建地形项（白色边框）
    QGraphicsPathItem *terrainItem = new QGraphicsPathItem(path);
    terrainItem->setBrush(QBrush(QColor(240, 240, 240))); // 雪地颜色
    terrainItem->setPen(QPen(QColor(240, 240, 240), 2)); // 竖直和底部边框设为白色
    terrainItem->setPos(chunkIndex * CHUNK_WIDTH, 0);

    // 创建顶部曲线路径（黑色边框）
    QPainterPath topPath;
    topPath.moveTo(points.first());
    for (int i = 1; i < points.size(); ++i) {
        topPath.lineTo(points[i]);
    }

    QGraphicsPathItem *topItem = new QGraphicsPathItem(topPath);
    topItem->setPen(QPen(Qt::black, 2)); // 顶部曲线保持黑色
    topItem->setPos(chunkIndex * CHUNK_WIDTH, 0);    // 添加到场景中
    m_scene->addItem(terrainItem);
    m_scene->addItem(topItem);
    m_chunks[chunkIndex] = terrainItem;
    m_topLineItems[chunkIndex] = topItem;

    // 生成2~3个石头
    int rockCount = QRandomGenerator::global()->bounded(2, 4);
    QVector<qreal> rockXs; // 记录已生成石头的x坐标
    for (int r = 0; r < rockCount; ++r) {
        // 随机x坐标（块内）
        qreal x = QRandomGenerator::global()->bounded(0, CHUNK_WIDTH);
        // 检查与已生成石头的距离
        bool tooClose = false;
        for (qreal prevX : rockXs) {
            if (qAbs(x - prevX) < MIN_ROCK_DISTANCE) {
                tooClose = true;
                break;
            }
        }
        if (tooClose) continue;

        qreal globalX = chunkIndex * CHUNK_WIDTH + x;
        // 检查斜率
        qreal slope = getTerrainSlope(globalX);
        if (qAbs(slope) > MAX_SLOPE_FOR_ROCK || chunkIndex == 0) continue;

        qreal y = getTerrainHeight(globalX) - 30; // 石头底部贴地

        RockEntity* rock = new RockEntity(30, 30);
        rock->setPosition(QPointF(globalX, y));
        qreal angle = qAtan(slope) * 180.0 / M_PI;
        rock->setRotation(angle);
        rock->setOnGround(true);
        m_scene->addItem(rock);
        m_rocks.append(rock);
        rockXs.append(x); // 记录本次石头x        // 注册到物理系统
        PhysicsSystem::instance().registerObject(rock);
    }
      // 生成NPC (企鹅:雪怪 = 4:1比例)
    if (chunkIndex > 0) { // 跳过第一个地形块
        // 随机选择生成位置（块内）
        qreal npcX = QRandomGenerator::global()->bounded(CHUNK_WIDTH / 4, CHUNK_WIDTH * 3 / 4);
        qreal globalNpcX = chunkIndex * CHUNK_WIDTH + npcX;
        
        // 检查斜率是否适合生成NPC
        qreal npcSlope = getTerrainSlope(globalNpcX);
        if (qAbs(npcSlope) <= MAX_SLOPE_FOR_ROCK) { // 使用与石头相同的斜率限制
            // 使用与Player相同的定位逻辑：terrainHeight - 完整高度
            qreal npcY = getTerrainHeight(globalNpcX) - 30; // 统一使用30像素偏移，与NPC高度一致
            
            // 决定生成企鹅还是雪怪 (4:1比例)
            // 每5个chunk为一个周期，其中4个生成企鹅，1个生成雪怪
            int cyclePosition = chunkIndex % 5;
            bool shouldGenerateYeti = (cyclePosition == 0); // 每5个chunk的第1个生成雪怪
            
            if (shouldGenerateYeti) {
                // 创建雪怪NPC
                auto yeti = NPCFactory::createYetiNPC(QPointF(globalNpcX, npcY));
                NPCEntity* yetiPtr = yeti.release();
                
                // 设置NPC初始状态
                yetiPtr->setOnGround(true);
                yetiPtr->setActive(false);
                
                // 添加到场景和存储列表
                m_scene->addItem(yetiPtr);
                m_npcs.append(yetiPtr);
                
                // 注册到物理系统
                PhysicsSystem::instance().registerObject(yetiPtr);
                
                DEBUG_LOG(QString("Generated yeti NPC at chunk %1, position (%2, %3)")
                          .arg(chunkIndex).arg(globalNpcX).arg(npcY));
            } else {
                // 创建企鹅NPC
                auto penguin = NPCFactory::createPenguinNPC(QPointF(globalNpcX, npcY));
                NPCEntity* penguinPtr = penguin.release(); // 释放unique_ptr的所有权
                
                // 设置NPC初始状态
                penguinPtr->setOnGround(true);
                penguinPtr->setActive(false); // 初始状态不激活
                
                // 添加到场景和存储列表
                m_scene->addItem(penguinPtr);
                m_npcs.append(penguinPtr);
                
                // 注册到物理系统
                PhysicsSystem::instance().registerObject(penguinPtr);
                
                DEBUG_LOG(QString("Generated penguin NPC at chunk %1, position (%2, %3)")
                          .arg(chunkIndex).arg(globalNpcX).arg(npcY));
            }
        }
    }
}

// 添加头文件
#include <QMutexLocker>

// 线程安全的区块生成方法
void TerrainGenerator::generateChunkThreadSafe(int chunkIndex)
{
    // 检查是否已经存在
    {
        QMutexLocker locker(&m_mutex);
        if (m_chunks.contains(chunkIndex)) {
            return;
        }
    }

    // 定义地形参数
    const int POINTS = 2000; // 每个地形块上的点数量
    const int BASE_HEIGHT = 300; // 地基高度
    const int HEIGHT_VARIATION = 10; // 高度变化范围
    const int BASE_SLOPE_FACTOR = 1400; // 基本下降趋势因子
    const int TRANSITION_ZONE = 400; // 两侧过渡区域的点数

    // 创建地形点
    QVector<QPointF> points;

    // 确保与前一个块平滑连接
    qreal startHeight = BASE_HEIGHT;
    qreal startSlope = 0.0; 
    qreal SLOPE_FACTOR = BASE_SLOPE_FACTOR;
    
    // 从原始的generateChunk方法复制的地形生成核心代码
    { // 访问 m_chunkPoints 需要加锁
        QMutexLocker locker(&m_mutex);
        if (chunkIndex > 0 && m_chunkPoints.contains(chunkIndex - 1)) {
            const QVector<QPointF> &prevPoints = m_chunkPoints[chunkIndex - 1];
            if (!prevPoints.isEmpty()) {
                startHeight = prevPoints.last().y();
                if (prevPoints.size() >= 2) {
                    qreal lastDelta = prevPoints.last().y() - prevPoints[prevPoints.size() - 2].y();
                    qreal lastDx = prevPoints.last().x() - prevPoints[prevPoints.size() - 2].x();
                    if (qAbs(lastDx) > 1e-9) { // 避免除以零
                        startSlope = lastDelta / lastDx;
                    }
                }
            }
        } else if (chunkIndex < 0 && m_chunkPoints.contains(chunkIndex + 1)) {
            const QVector<QPointF> &nextPoints = m_chunkPoints[chunkIndex + 1];
            if (!nextPoints.isEmpty()) {
                startHeight = nextPoints.first().y();
                if (nextPoints.size() >= 2) {
                    qreal firstDelta = nextPoints[1].y() - nextPoints[0].y();
                    qreal firstDx = nextPoints[1].x() - nextPoints[0].x();
                    if (qAbs(firstDx) > 1e-9) { // 避免除以零
                        startSlope = firstDelta / firstDx;
                    }
                }
            }
        }
    }

    // 第一个点
    points.append(QPointF(0, startHeight));

    // 生成随机地形点
    for (int i = 1; i < POINTS; ++i) {
        qreal x = (qreal) i / POINTS * CHUNK_WIDTH;
        qreal globalX = x + chunkIndex * CHUNK_WIDTH;

        qreal noiseValue = noise(globalX * 1); 

        qreal globalFactor = chunkIndex * BASE_SLOPE_FACTOR; 
        qreal localFactor = qSqrt((qreal) i / POINTS) * SLOPE_FACTOR; 
        qreal downwardTrend = globalFactor + localFactor;

        qreal baseHeight = BASE_HEIGHT + downwardTrend;
        qreal blendFactor = 1.0;
        
        if (i < TRANSITION_ZONE) {
            qreal t = (qreal) i / TRANSITION_ZONE;
            qreal smoothT = (1 - qCos(t * M_PI)) * 0.5; 

            qreal expectedHeight = startHeight + startSlope * x;

            qreal transitionNoise = noise(globalX * 0.02 + 100) * HEIGHT_VARIATION * 0.3;
            expectedHeight += transitionNoise * smoothT; 

            baseHeight = expectedHeight * (1 - smoothT) + baseHeight * smoothT;

            noiseValue *= smoothT;
        }

        qreal height = baseHeight + noiseValue * HEIGHT_VARIATION * blendFactor;
        points.append(QPointF(x, height));
    }
    
    // 创建地形路径
    QPainterPath path;
    if (points.isEmpty()) {
        // 这种情况理论上不应该发生，因为至少会添加一个起始点
        qWarning() << "TerrainGenerator::generateChunkThreadSafe - points vector is unexpectedly empty for chunkIndex:" << chunkIndex;
        // 为避免崩溃，存储空路径和点
        QMutexLocker locker(&m_mutex);
        m_generatedPaths[chunkIndex] = path;
        m_chunkPoints[chunkIndex] = points;
        return;
    }
    path.moveTo(points.first());

    // 添加所有点
    for (int i = 1; i < points.size(); ++i) {
        path.lineTo(points[i]);
    }    // 完成地形封闭
    path.lineTo(CHUNK_WIDTH, 5000000);
    path.lineTo(0, 5000000);
    path.closeSubpath();
      // 生成石头的位置数据（不创建实体）
    QVector<RockGenerationData> rockDataList;
    int rockCount = QRandomGenerator::global()->bounded(2, 4);
    QVector<qreal> rockXs; // 记录已生成石头的x坐标
    
    for (int r = 0; r < rockCount; ++r) {
        // 随机x坐标（块内）
        qreal x = QRandomGenerator::global()->bounded(0, CHUNK_WIDTH);
        // 检查与已生成石头的距离
        bool tooClose = false;
        for (qreal prevX : rockXs) {
            if (qAbs(x - prevX) < MIN_ROCK_DISTANCE) {
                tooClose = true;
                break;
            }
        }
        if (tooClose) continue;

        // 直接使用刚生成的points数据计算高度和斜率
        // 找到x坐标最接近的两个点
        int i = 0;
        while (i < points.size() && points[i].x() < x) {
            i++;
        }
        
        // 如果没找到合适的点或位置无效，则跳过生成这个石头
        if (i == 0 || i >= points.size()) {
            continue;
        }
          // 计算斜率
        qreal x1 = points[i - 1].x();
        qreal y1 = points[i - 1].y();
        qreal x2 = points[i].x();
        qreal y2 = points[i].y();
        
        // 避免除零错误
        qreal dx = x2 - x1;
        qreal slope = (qAbs(dx) > 1e-9) ? ((y2 - y1) / dx) : 0.0;
        
        // 检查斜率，如果太陡则不放置石头
        if (qAbs(slope) > MAX_SLOPE_FOR_ROCK) continue;
        
        // 计算高度（线性插值）
        qreal height = y1 + slope * (x - x1);
        qreal y = height - 30; // 石头底部贴地
        
        qreal globalX = chunkIndex * CHUNK_WIDTH + x;
        qreal angle = qAtan(slope) * 180.0 / M_PI;
        
        // 保存石头数据
        RockGenerationData rockData;
        rockData.localX = x;
        rockData.globalX = globalX;
        rockData.y = y;
        rockData.angle = angle;        rockDataList.append(rockData);
        rockXs.append(x); // 记录本次石头x
    }
    
    // 生成企鹅NPC的位置数据（不创建实体）
    QVector<NPCGenerationData> npcDataList;
    if (chunkIndex > 0) { // 跳过第一个地形块
        // 随机选择生成位置（块内）
        qreal npcX = QRandomGenerator::global()->bounded(CHUNK_WIDTH / 4, CHUNK_WIDTH * 3 / 4);
        
        // 计算高度和斜率
        int i = 0;
        while (i < points.size() && points[i].x() < npcX) {
            i++;
        }
        
        if (i > 0 && i < points.size()) {
            // 计算斜率
            qreal x1 = points[i - 1].x();
            qreal y1 = points[i - 1].y();
            qreal x2 = points[i].x();
            qreal y2 = points[i].y();
            
            qreal dx = x2 - x1;
            qreal slope = (qAbs(dx) > 1e-9) ? ((y2 - y1) / dx) : 0.0;
            
            // 检查斜率是否适合生成NPC
            if (qAbs(slope) <= MAX_SLOPE_FOR_ROCK) {
                // 计算高度（线性插值）
                qreal height = y1 + slope * (npcX - x1);
                qreal npcY = height - 15; // NPC高度的一半
                qreal globalNpcX = chunkIndex * CHUNK_WIDTH + npcX;
                
                // 保存NPC数据
                NPCGenerationData npcData;
                npcData.localX = npcX;
                npcData.globalX = globalNpcX;
                npcData.y = npcY;
                npcData.type = NPCEntity::NPCType::Ground; // 企鹅是地面类型
                npcDataList.append(npcData);
            }
        }
    }
      // 存储生成的路径、点数据、石头数据和NPC数据
    {
        QMutexLocker locker(&m_mutex);
        m_generatedPaths[chunkIndex] = path;
        m_chunkPoints[chunkIndex] = points;
        m_generatedRocks[chunkIndex] = rockDataList;
        m_generatedNPCs[chunkIndex] = npcDataList;
    }
}

// 在主线程中完成将区块添加到场景
void TerrainGenerator::addChunkToScene(int chunkIndex)
{
    QMutexLocker locker(&m_mutex);
    
    // 检查是否已经添加到场景
    if (m_chunks.contains(chunkIndex)) {
        return;
    }
    
    // 检查是否有生成的路径
    if (!m_generatedPaths.contains(chunkIndex)) {
        return;
    }
    
    QPainterPath path = m_generatedPaths[chunkIndex];
      // 创建地形项（白色填充）
    QGraphicsPathItem *terrainItem = new QGraphicsPathItem(path);
    terrainItem->setBrush(QBrush(QColor(240, 240, 240))); // 雪地颜色
    terrainItem->setPen(QPen(QColor(240, 240, 240), 2)); // 竖直和底部边框设为白色
    terrainItem->setPos(chunkIndex * CHUNK_WIDTH, 0);
    
    // 添加地形项到场景
    m_scene->addItem(terrainItem);
    m_chunks[chunkIndex] = terrainItem;
    
    // 创建顶部曲线路径（黑色边框）
    QGraphicsPathItem *topItem = nullptr;
    if (m_chunkPoints.contains(chunkIndex)) {
        const QVector<QPointF>& points = m_chunkPoints[chunkIndex];
        QPainterPath topPath;
        topPath.moveTo(points.first());
        for (int i = 1; i < points.size(); ++i) {
            topPath.lineTo(points[i]);
        }
        
        topItem = new QGraphicsPathItem(topPath);
        topItem->setPen(QPen(Qt::black, 2)); // 顶部曲线保持黑色
        topItem->setPos(chunkIndex * CHUNK_WIDTH, 0);
          // 添加轮廓线到场景并保存
        m_scene->addItem(topItem);
        m_topLineItems[chunkIndex] = topItem;
    }
    
    // 根据预先计算的数据创建石头实体
    if (m_generatedRocks.contains(chunkIndex)) {
        const QVector<RockGenerationData>& rockDataList = m_generatedRocks[chunkIndex];
        for (const RockGenerationData& rockData : rockDataList) {
            RockEntity* rock = new RockEntity(30, 30);
            rock->setPosition(QPointF(rockData.globalX, rockData.y));
            rock->setRotation(rockData.angle);
            rock->setOnGround(true);
            m_scene->addItem(rock);
            m_rocks.append(rock);
            // 注册到物理系统
            PhysicsSystem::instance().registerObject(rock);
        }
          // 处理完后移除石头数据
        m_generatedRocks.remove(chunkIndex);
    }
    
    // 根据预先计算的数据创建NPC实体
    if (m_generatedNPCs.contains(chunkIndex)) {
        const QVector<NPCGenerationData>& npcDataList = m_generatedNPCs[chunkIndex];
        for (const NPCGenerationData& npcData : npcDataList) {
            // 目前只创建企鹅NPC
            auto penguin = NPCFactory::createPenguinNPC(QPointF(npcData.globalX, npcData.y));
            NPCEntity* penguinPtr = penguin.release(); // 释放unique_ptr的所有权
            
            // 设置NPC初始状态
            penguinPtr->setOnGround(true);
            penguinPtr->setActive(false); // 初始状态不激活，等待进入画面
            
            // 添加到场景和存储列表
            m_scene->addItem(penguinPtr);
            m_npcs.append(penguinPtr);
            
            // 注册到物理系统
            PhysicsSystem::instance().registerObject(penguinPtr);
            
            DEBUG_LOG(QString("Created penguin NPC at chunk %1, position (%2, %3)")
                      .arg(chunkIndex).arg(npcData.globalX).arg(npcData.y));
        }
        
        // 处理完后移除NPC数据
        m_generatedNPCs.remove(chunkIndex);
    }
    
    // 移除已处理的路径
    m_generatedPaths.remove(chunkIndex);
}

void TerrainGenerator::removeDistantChunks(int currentChunk) {
    QList<int> chunksToRemove;

    // 查找并删除远离的块
    for (auto it = m_chunks.begin(); it != m_chunks.end(); ++it) {
        if (qAbs(it.key() - currentChunk) > VIEW_CHUNKS * 2) {
            chunksToRemove.append(it.key());
        }
    }    // 添加调试信息
    if (!chunksToRemove.isEmpty()) {
        DEBUG_LOG(QString("移除 %1 个远距离地形块，当前块索引: %2").arg(chunksToRemove.size()).arg(currentChunk));
        QStringList chunkIndexStrings;
        for (int idx : chunksToRemove) {
            chunkIndexStrings << QString::number(idx);
        }
        DEBUG_LOG(QString("被移除的块索引: %1").arg(chunkIndexStrings.join(", ")));
    }

    // 从场景和映射中删除
    // 在移除地形块时，移除该块内的石头
    for (int index : chunksToRemove) {
        // 移除石头
        for (int i = m_rocks.size() - 1; i >= 0; --i) {
            RockEntity* rock = m_rocks[i];
            if (rock->x() >= index * CHUNK_WIDTH && rock->x() < (index + 1) * CHUNK_WIDTH) {
                m_scene->removeItem(rock);
                PhysicsSystem::instance().unregisterObject(rock);
                delete rock;
                m_rocks.remove(i);
            }
        }        // 移除地形块
        if (m_chunks.contains(index)) {
            m_scene->removeItem(m_chunks[index]);
            delete m_chunks[index];
            m_chunks.remove(index);
        }
        
        // 移除轮廓线
        if (m_topLineItems.contains(index)) {
            m_scene->removeItem(m_topLineItems[index]);
            delete m_topLineItems[index];
            m_topLineItems.remove(index);
        }
        
        // 保留地形点数据，因为可能需要用于连接
    }
}

qreal TerrainGenerator::noise(qreal x) const {
    // 根据需要调整频率（0.005）和振幅（HEIGHT_VARIATION）
    double value = m_perlin.noise(x * 0.005);
    return value * 2.0 - 1.0; // 映射到 [-1,1]
}

qreal TerrainGenerator::getTerrainSlope(qreal x) const {
    // 计算点所在的地形块
    int chunkIndex = floor(x / CHUNK_WIDTH);
    qreal localX = x - chunkIndex * CHUNK_WIDTH;

    if (!m_chunkPoints.contains(chunkIndex)) {
        return 0;
    }

    const QVector<QPointF> &points = m_chunkPoints[chunkIndex];

    // 找到x坐标最接近的两个点
    int i = 0;
    while (i < points.size() && points[i].x() < localX) {
        i++;
    }

    if (i <= 0 || i >= points.size()) {
        return 0;
    }

    // 计算斜率
    qreal x1 = points[i - 1].x();
    qreal y1 = points[i - 1].y();
    qreal x2 = points[i].x();
    qreal y2 = points[i].y();

    return (y2 - y1) / (x2 - x1);
}

// 在析构函数中停止线程
TerrainGenerator::~TerrainGenerator() {
    if (m_generatorThread) {
        m_generatorThread->stop();
        m_generatorThread->wait();
    }
}

void TerrainGenerator::clearAllResources()
{
    // 停止线程
    if (m_generatorThread) {
        m_generatorThread->stop();
        m_generatorThread->wait();
    }
      // 清理地形块
    for (auto it = m_chunks.begin(); it != m_chunks.end(); ++it) {
        if (it.value()) {
            m_scene->removeItem(it.value());
            delete it.value();
        }
    }
    m_chunks.clear();
    
    // 清理轮廓线
    for (auto it = m_topLineItems.begin(); it != m_topLineItems.end(); ++it) {
        if (it.value()) {
            m_scene->removeItem(it.value());
            delete it.value();
        }
    }
    m_topLineItems.clear();
    
    // 清理石头
    for (RockEntity* rock : m_rocks) {
        if (rock) {
            m_scene->removeItem(rock);
            PhysicsSystem::instance().unregisterObject(rock);
            delete rock;
        }
    }
    m_rocks.clear();
    
    // 清理其他数据
    m_chunkPoints.clear();
    m_generatedPaths.clear();
    m_generatedRocks.clear();
}
