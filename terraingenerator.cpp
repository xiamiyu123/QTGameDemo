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
    
    // 生成前方的地形块
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
        return 300;
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
    const int POINTS = 50; // 每个地形块上的点数量
    const int BASE_HEIGHT = 300; // 基础高度
    const int HEIGHT_VARIATION = 100; // 高度变化范围
    
    // 创建地形点
    QVector<QPointF> points;
    
    // 确保与前一个块平滑连接
    qreal startHeight = BASE_HEIGHT;
    if (chunkIndex > 0 && m_chunkPoints.contains(chunkIndex - 1)) {
        startHeight = m_chunkPoints[chunkIndex - 1].last().y();
    } else if (chunkIndex < 0 && m_chunkPoints.contains(chunkIndex + 1)) {
        startHeight = m_chunkPoints[chunkIndex + 1].first().y();
    }
    
    // 第一个点
    points.append(QPointF(0, startHeight));
    
    // 生成随机地形点
    for (int i = 1; i < POINTS; ++i) {
        qreal x = (qreal)i / POINTS * CHUNK_WIDTH;
        
        // 使用简化的噪声函数生成自然高度
        qreal noiseValue = noise(x + chunkIndex * CHUNK_WIDTH);
        qreal height = BASE_HEIGHT + noiseValue * HEIGHT_VARIATION;
        
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
    path.lineTo(CHUNK_WIDTH, 600);
    path.lineTo(0, 600);
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
    // 简单的噪声函数，实际项目中可使用Perlin噪声等更高级算法
    return qSin(x * 0.01) + 
           qSin(x * 0.02 + 0.3) * 0.5 + 
           qSin(x * 0.03 + 0.6) * 0.25;
}