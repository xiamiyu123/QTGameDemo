#ifndef TERRAINGENERATOR_H
#define TERRAINGENERATOR_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QMap>
#include <QVector>
#include <QRandomGenerator>

class TerrainGenerator : public QObject
{
    Q_OBJECT
    
public:
    TerrainGenerator(QGraphicsScene *scene, QObject *parent = nullptr);
    
    // 初始化地形生成
    void initialize();
    
    // 更新地形（基于玩家位置）
    void updateTerrain(qreal playerX);
    
    // 获取地形高度
    qreal getTerrainHeight(qreal x) const;
    
private:
    static const int CHUNK_WIDTH = 800;    // 地形块宽度
    static const int VIEW_CHUNKS = 3;      // 视图范围内保持的地形块数量
    
    QGraphicsScene *m_scene;
    QMap<int, QGraphicsPathItem*> m_chunks; // 当前显示的地形块
    QMap<int, QVector<QPointF>> m_chunkPoints; // 每个地形块的关键点
    
    int m_seed;  // 随机种子
    QRandomGenerator m_randomGenerator; // 随机数生成器
    
    // 生成地形块
    void generateChunk(int chunkIndex);
    
    // 删除远离的地形块
    void removeDistantChunks(int currentChunk);
    
    // 获取噪声值
    qreal noise(qreal x) const;
};

#endif // TERRAINGENERATOR_H