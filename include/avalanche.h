#pragma once
#include <QGraphicsPathItem>
#include "terraingenerator.h"

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
    bool isPlayerCaught(qreal playerX) const;
    // 获取雪崩前沿与玩家的距离
    qreal distanceToPlayer(qreal playerX) const;
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
};
