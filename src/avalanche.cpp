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
      m_width(6000),        // 雪崩宽度
      m_visibleRange(2000)  // 默认视距范围
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

    // 检查雪崩是否在玩家视野范围内
    if (m_frontX + m_width < playerX - m_visibleRange) {
        // 完全在视野左侧外，隐藏
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

void Avalanche::setVisibleRange(qreal range)
{
    m_visibleRange = range;
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
    QPainterPath path;

    // 只绘制可见区域内的雪崩部分
    qreal leftLimit = qMax(m_frontX - m_width, playerX - m_visibleRange * 1.2);
    qreal rightLimit = qMin(m_frontX, playerX + m_visibleRange * 0.5);
    
    // 如果没有可见部分，返回空路径
    if (leftLimit >= rightLimit) {
        setPath(path);
        return;
    }
    
    // 动态计算需要的点数，基于可见雪崩宽度
    qreal visibleWidth = rightLimit - leftLimit;
    int points = qMin(200, qMax(50, int(visibleWidth / 30)));
    qreal step = visibleWidth / (points - 1);

    QVector<QPointF> topPoints, bottomPoints;
    for (int i = 0; i < points; ++i) {
        qreal x = leftLimit + i * step;
        qreal y = m_terrain->getTerrainHeight(x);
        qreal slope = m_terrain->getTerrainSlope(x);
        qreal topY = y - 60 - slope * 40;
        topPoints.append(QPointF(x, topY));
        bottomPoints.append(QPointF(x, y));
    }

    // 只在雪崩前沿位于可见区域时绘制半圆
    if (rightLimit > m_frontX - 10) {
        // 右端半圆参数
        qreal groundY = m_terrain->getTerrainHeight(m_frontX);
        qreal topY = groundY - 60 - m_terrain->getTerrainSlope(m_frontX) * 40;
        qreal radius = groundY - topY; // 顶部到地面的距离
        QPointF arcCenter(m_frontX, groundY);

        QVector<QPointF> arcPoints;
        const int arcSegments = 20; // 减少半圆的段数
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
    } else {
        // 雪崩前沿在视野外，简化为矩形
        path.moveTo(topPoints.first());
        for (const auto& pt : topPoints) path.lineTo(pt);
        path.lineTo(topPoints.last().x(), bottomPoints.last().y());
    }
    
    // 完成底部路径
    for (int i = bottomPoints.size() - 1; i >= 0; --i)
        path.lineTo(bottomPoints[i]);
    path.closeSubpath();

    setPath(path);
}

// 添加线程安全方法实现

void Avalanche::updateAvalancheThreadSafe(qreal elapsed, qreal playerX)
{
    // 复制原来的updateAvalanche逻辑，但不直接修改图形项
    // 仅进行计算并存储结果
    
    QVector<QPointF> newPositions;
    
    // 雪崩更新的核心逻辑
    // ...计算过程...
    
    // 将结果存储到线程安全的缓冲区
    QMutexLocker locker(&m_mutex);
    m_threadCalculatedPositions = newPositions;
    m_hasThreadResults = true;
}

void Avalanche::applyThreadResults()
{
    QMutexLocker locker(&m_mutex);
    if (m_hasThreadResults) {
        // 应用线程计算的结果到实际图形项
        // 这部分在主线程中执行
        
        // ...更新图形项...
        
        m_hasThreadResults = false;
    }
}
