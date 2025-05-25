#pragma once
#include <QGraphicsPathItem>
#include "terraingenerator.h"
#include <QMutex>

class Avalanche : public QGraphicsPathItem
{
public:
    Avalanche(TerrainGenerator* terrain, QGraphicsItem* parent = nullptr);

    void updateAvalanche(qreal deltaTime, qreal playerX);

    qreal getFrontX() const;

    void setSpeed(qreal speed);
    void setAcceleration(qreal acc);
    void setMaxSpeed(qreal maxSpeed);
    void setVisibleRange(qreal range);
    //判断玩家是否被雪崩追上
    bool isPlayerCaught(qreal playerX);
    //判断玩家是否被雪崩超越
    bool isPlayerSurpassed(qreal playerX);
    // 获取雪崩前沿与玩家的距离
    qreal distanceToPlayer(qreal playerX);
    // 获取雪崩末端与玩家的距离
    qreal distanceToPlayerLeft(qreal playerX);

    // 线程安全的雪崩更新方法
    void updateAvalancheThreadSafe(qreal elapsed, qreal playerX);

    // 将线程中计算的结果应用到主线程
    void applyThreadResults();

private:
    TerrainGenerator* m_terrain;
    qreal m_frontX;      // 雪崩前沿x坐标
    qreal m_speed;       // 当前速度
    qreal m_acceleration;// 加速度
    qreal m_maxSpeed;    // 最大速度
    qreal m_width;       // 雪崩宽度
    qreal m_visibleRange;// 玩家视距范围

    void updateShape(qreal playerX);
    bool isInPlayerView(qreal playerX) const;

    QMutex m_mutex;  // 保护共享数据的互斥锁

    // 线程计算结果的临时存储
    qreal m_threadCalculatedFrontX;
    qreal m_threadCalculatedSpeed;
    QVector<QPointF> m_threadCalculatedTopPoints;
    QVector<QPointF> m_threadCalculatedBottomPoints;
    QVector<QPointF> m_threadCalculatedArcPoints;
    bool m_threadCalculatedVisible;
    bool m_hasThreadResults = false;
};
