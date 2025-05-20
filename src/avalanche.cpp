#include "avalanche.h"
#include <QBrush>
#include <QPen>
#include <QtMath>

Avalanche::Avalanche(TerrainGenerator* terrain, QGraphicsItem* parent)
    : QGraphicsPathItem(parent),
      m_terrain(terrain),
      m_frontX(0),
      m_speed(200),         // 初始速度
      m_acceleration(10),   // 加速度
      m_maxSpeed(650),      // 最大速度（比玩家快）
      m_width(4000)          // 雪崩宽度
{
    setZValue(-1);
    setBrush(QBrush(QColorConstants::Svg::white)); // 雪崩颜色
    setPen(QPen(QColorConstants::Svg::white));
}

void Avalanche::updateAvalanche(qreal deltaTime, qreal playerX)
{
    // 雪崩随时间加速
    m_speed += m_acceleration * deltaTime;
    if (m_speed > m_maxSpeed) m_speed = m_maxSpeed;

    // 雪崩前沿推进
    m_frontX += m_speed * deltaTime;

    // 雪崩尾部自动清理
    if (m_frontX + m_width < playerX - 1200) {
        // 超出视野，隐藏或删除
        setVisible(false);
        return;
    } else {
        setVisible(true);
    }

    updateShape(playerX);
}

qreal Avalanche::getFrontX() const
{
    return m_frontX;
}
void Avalanche::setSpeed(qreal speed)
{
    m_speed = speed;
}
void Avalanche::setAcceleration(qreal acc)
{
    m_acceleration = acc;
}
void Avalanche::setMaxSpeed(qreal maxSpeed)
{
    m_maxSpeed = maxSpeed;
}
bool Avalanche::isPlayerCaught(qreal playerX) const
{
    return distanceToPlayer(playerX) < 100;
}
qreal Avalanche::distanceToPlayer(qreal playerX) const
{
    return qAbs(playerX - m_frontX);
}



void Avalanche::updateShape(qreal playerX)
{
    const int points = 200;
    QPainterPath path;

    qreal leftX = m_frontX - m_width;
    qreal rightX = m_frontX;
    qreal step = (rightX - leftX) / (points - 1);

    QVector<QPointF> topPoints, bottomPoints;
    for (int i = 0; i < points; ++i) {
        qreal x = leftX + i * step;
        qreal y = m_terrain->getTerrainHeight(x);
        qreal slope = m_terrain->getTerrainSlope(x);
        qreal topY = y - 60 - slope * 40;
        topPoints.append(QPointF(x, topY));
        bottomPoints.append(QPointF(x, y));
    }

    // 右端半圆参数
    qreal groundY = bottomPoints.last().y();
    qreal topY = topPoints.last().y();
    qreal radius = groundY - topY; // 顶部到地面的距离
    QPointF arcCenter(rightX, groundY);

    QVector<QPointF> arcPoints;
    const int arcSegments = 40;
    for (int i = 0; i <= arcSegments; ++i) {
        // 角度从270°到90°（顺时针，从上到下）
        qreal theta = M_PI * 1.5 + M_PI * (i / (qreal)arcSegments);
        qreal x = arcCenter.x() + radius * qCos(theta);
        qreal y = arcCenter.y() + radius * qSin(theta);
        arcPoints.append(QPointF(x, y));
    }

    // 构造路径
    path.moveTo(topPoints.first());
    for (const auto& pt : topPoints) path.lineTo(pt);
    for (const auto& pt : arcPoints) path.lineTo(pt);
    for (int i = bottomPoints.size() - 1; i >= 0; --i)
        path.lineTo(bottomPoints[i]);
    path.closeSubpath();

    setPath(path);
}