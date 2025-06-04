#pragma once

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QMap>
#include <QVector>
#include <QRandomGenerator>
#include <QMutex>
#include "perlinnoise.h"
#include "terraingeneratorthread.h"
#include "rockentity.h"
#include "npcentity.h"
#include "treeentity.h"
#include "cloudentity.h"
#include <QVector>

// 石头数据结构，存储石头的位置信息
struct RockGenerationData {
    qreal localX;      // 块内的X坐标
    qreal globalX;     // 全局X坐标
    qreal y;           // Y坐标
    qreal angle;       // 旋转角度
};

// NPC数据结构，存储NPC的位置信息
struct NPCGenerationData {
    qreal localX;      // 块内的X坐标
    qreal globalX;     // 全局X坐标
    qreal y;           // Y坐标
    int npcId;         // NPC类型ID（1=企鹅，2=雪人）
};

// 树木数据结构，存储树木的位置信息
struct TreeGenerationData {
    qreal localX;      // 块内的X坐标
    qreal globalX;     // 全局X坐标
    qreal y;           // Y坐标
    qreal width;       // 树木宽度
    qreal height;      // 树木高度
};

// 云朵数据结构，存储云朵的位置信息
struct CloudGenerationData {
    qreal localX;      // 块内的X坐标
    qreal globalX;     // 全局X坐标
    qreal y;           // Y坐标
    qreal width;       // 云朵宽度
    qreal height;      // 云朵高度
};

class TerrainGenerator : public QObject
{
    Q_OBJECT

public:
    TerrainGenerator(QGraphicsScene *scene, QObject *parent = nullptr);
    ~TerrainGenerator();
    // 初始化地形生成
    void initialize();
    
    // 更新地形（基于玩家位置）
    void updateTerrain(qreal playerX);
    
    // 获取指定位置地形高度
    qreal getTerrainHeight(qreal x) const;

    // 获取指定位置的地形坡度（返回斜率值）
    qreal getTerrainSlope(qreal x) const;

    // 线程安全的区块生成方法
    void generateChunkThreadSafe(int chunkIndex);    // 在主线程中完成将区块添加到场景的操作
    void addChunkToScene(int chunkIndex);
    QVector<RockEntity*> m_rocks; // 存储所有石头 - 移到public部分
    QVector<NPCEntity*> m_npcs; // 存储所有NPC
QVector<TreeEntity*> m_trees; // 存储所有树木
    QVector<CloudEntity*> m_clouds; // 存储所有云朵

    // 添加清理方法
    void clearAllResources();
    
private:
    static const int CHUNK_WIDTH = 3600;    // 地形块宽度
    static const int VIEW_CHUNKS = 1;      // 视图范围内保持的地形块数量
    QGraphicsScene *m_scene;
    QMap<int, QGraphicsPathItem*> m_chunks; // 当前显示的地形块
    QMap<int, QGraphicsPathItem*> m_topLineItems; // 地形块的顶部轮廓线
    QMap<int, QVector<QPointF>> m_chunkPoints; // 每个地形块的关键点
    
    int m_seed;  // 随机种子
    QRandomGenerator m_randomGenerator; // 随机数生成器

    // 互斥锁，保护共享资源
    mutable QMutex m_mutex;    // 后台线程生成的区块数据
    QMap<int, QPainterPath> m_generatedPaths;
      // 后台线程生成的石头数据
    QMap<int, QVector<RockGenerationData>> m_generatedRocks;
      // 后台线程生成的NPC数据
    QMap<int, QVector<NPCGenerationData>> m_generatedNPCs;

    // 后台线程生成的树木数据
    QMap<int, QVector<TreeGenerationData>> m_generatedTrees;

    // 后台线程生成的云朵数据
    QMap<int, QVector<CloudGenerationData>> m_generatedClouds;

    // 生成地形块
    void generateChunk(int chunkIndex);

    PerlinNoise m_perlin;

    // 删除远离的地形块
    void removeDistantChunks(int currentChunk);
    
    // 获取噪声值
    qreal noise(qreal x) const;

    TerrainGeneratorThread* m_generatorThread;
};

