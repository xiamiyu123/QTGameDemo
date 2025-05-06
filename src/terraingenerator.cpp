#include "terraingenerator.h"
#include <QDateTime>
#include <QPen>
#include <QBrush>
#include <QtMath>
#include <QRandomGenerator>
#include <QDebug>

TerrainGenerator::TerrainGenerator(QGraphicsScene *scene, QObject *parent)
    : QObject(parent),
      m_scene(scene)
{
    // 初始化随机种子
    m_seed = QDateTime::currentMSecsSinceEpoch();
    m_randomGenerator = QRandomGenerator(m_seed);  // 创建自己的随机生成器实例
    m_perlin = PerlinNoise(m_seed); // 使用随机种子初始化Perlin噪声类
    // 初始化累计下降因子
    TOTAL_SLOPE_FACTOR = 0;
}
void TerrainGenerator::initialize()
{
    // 生成初始地形块
    for (int i = -1; i <= 1; ++i) {
        generateChunk(i);
    }
}

void TerrainGenerator::updateTerrain(qreal playerX)
{
    // 计算玩家当前所在的地形块
    int currentChunk = floor(playerX / CHUNK_WIDTH);

    // 满足条件时生成前方的地形块
    for (int i = currentChunk - 1; i <= currentChunk + VIEW_CHUNKS; ++i) {
        if (!m_chunks.contains(i)) {
            generateChunk(i);
        }
    }

    // 移除远离玩家的地形块
    removeDistantChunks(currentChunk);
}

qreal TerrainGenerator::getTerrainHeight(qreal x) const
{
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
    qreal x1 = points[i-1].x();
    qreal y1 = points[i-1].y();
    qreal x2 = points[i].x();
    qreal y2 = points[i].y();

    // 计算斜率和高度
    qreal slope = (y2 - y1) / (x2 - x1);
    return y1 + slope * (localX - x1);
}

void TerrainGenerator::generateChunk(int chunkIndex)
{
    if (m_chunks.contains(chunkIndex)) {
        return;
    }

    // 定义地形参数
    const int POINTS = 3000; // 每个地形块上的点数量
    const int BASE_HEIGHT = 300; // 地基高度
    const int HEIGHT_VARIATION = 50; // 高度变化范围
    const int BASE_SLOPE_FACTOR = 1200; // 基本下降趋势因子
    const int TRANSITION_ZONE = 200; // 两侧过渡区域的点数

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
        const QVector<QPointF>& prevPoints = m_chunkPoints[chunkIndex - 1];
        startHeight = prevPoints.last().y();

        // 计算前一块末尾的斜率
        if (prevPoints.size() >= 2) {
            qreal lastDelta = prevPoints.last().y() - prevPoints[prevPoints.size() - 2].y();
            qreal lastDx = prevPoints.last().x() - prevPoints[prevPoints.size() - 2].x();
            startSlope = lastDelta / lastDx;
        }
    } else if (chunkIndex < 0 && m_chunkPoints.contains(chunkIndex + 1)) {
        const QVector<QPointF>& nextPoints = m_chunkPoints[chunkIndex + 1];
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
    TOTAL_SLOPE_FACTOR += SLOPE_FACTOR;

    // 生成随机地形点
    for (int i = 1; i < POINTS; ++i) {
        qreal x = (qreal)i / POINTS * CHUNK_WIDTH;
        qreal globalX = x + chunkIndex * CHUNK_WIDTH;

        // 使用多层柏林噪声函数生成地形
        qreal noiseValue = noise(globalX * 1); // 因为一层就够好所以暂时只用一层

        // 平滑的下降趋势
        qreal downwardTrend = TOTAL_SLOPE_FACTOR + qSqrt((qreal)i / POINTS) * SLOPE_FACTOR;

        // 计算基础高度
        qreal baseHeight = BASE_HEIGHT + downwardTrend;
        // 添加随机变化
        // 在过渡区域内，使用权重混合来平滑过渡
        qreal blendFactor = 1.0;
        // 在过渡区域内，使用平滑函数（而非线性）混合来过渡
        if (i < TRANSITION_ZONE) {
            // 使用余弦插值函数替代线性插值，提供更自然的过渡
            qreal t = (qreal)i / TRANSITION_ZONE;
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

    // 创建地形项
    QGraphicsPathItem *terrainItem = new QGraphicsPathItem(path);
    terrainItem->setBrush(QBrush(QColor(240, 240, 240))); // 雪地颜色
    terrainItem->setPen(QPen(Qt::black, 2));
    terrainItem->setPos(chunkIndex * CHUNK_WIDTH, 0);

    // 添加到场景中
    m_scene->addItem(terrainItem);
    m_chunks[chunkIndex] = terrainItem;
}

void TerrainGenerator::removeDistantChunks(int currentChunk)
{
    QList<int> chunksToRemove;

    // 查找并删除远离的块
    for (auto it = m_chunks.begin(); it != m_chunks.end(); ++it) {
        if (qAbs(it.key() - currentChunk) > VIEW_CHUNKS) {
            chunksToRemove.append(it.key());
        }
    }

    // 从场景和映射中删除
    for (int index : chunksToRemove) {
        m_scene->removeItem(m_chunks[index]);
        delete m_chunks[index];
        m_chunks.remove(index);
        // 保留地形点数据，因为可能需要用于连接
    }
}

qreal TerrainGenerator::noise(qreal x) const
{
    // 根据需要调整频率（0.005）和振幅（HEIGHT_VARIATION）
    double value = m_perlin.noise(x * 0.005);
    return value * 2.0 - 1.0;  // 映射到 [-1,1]
}