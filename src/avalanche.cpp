#include "avalanche.h"
#include <QBrush>
#include <QPen>
#include <QtMath>

// 构造函数，初始化雪崩参数
Avalanche::Avalanche(TerrainGenerator* terrain, QGraphicsItem* parent)
    : QGraphicsPathItem(parent),                // 调用父类构造函数
      m_terrain(terrain),                       // 记录地形指针
      m_frontX(0),          // 雪崩前沿初始位置
      m_speed(200),         // 初始速度
      m_acceleration(10),   // 加速度
      m_maxSpeed(650),      // 最大速度
      m_width(4000)         // 雪崩宽度
{
    setZValue(-1); // 图层放在地形下方
    setBrush(QBrush(QColorConstants::Svg::white)); // 设置雪崩颜色为白色
    setPen(QPen(QColorConstants::Svg::white));     // 设置雪崩边框颜色为白色
}

// 雪崩每帧更新：推进前沿、加速、重绘形状
void Avalanche::updateAvalanche(qreal deltaTime, qreal playerX)
{
    m_speed += m_acceleration * deltaTime;      // 速度随时间增加
    if (m_speed > m_maxSpeed) m_speed = m_maxSpeed; // 限制最大速度

    m_frontX += m_speed * deltaTime;            // 雪崩前沿推进

    updateShape(playerX);                       // 更新雪崩形状
}

// 获取雪崩前沿x坐标
qreal Avalanche::getFrontX() const
{
    return m_frontX;
}

// 设置速度
void Avalanche::setSpeed(qreal speed)
{
    m_speed = speed;
}

// 设置加速度
void Avalanche::setAcceleration(qreal acc)
{
    m_acceleration = acc;
}

// 设置最大速度
void Avalanche::setMaxSpeed(qreal maxSpeed)
{
    m_maxSpeed = maxSpeed;
}

// 判断玩家是否被雪崩追上
bool Avalanche::isPlayerCaught(qreal playerX) const
{
    return distanceToPlayer(playerX) < 10; // 距离小于10判定为被追上
}

// 判断玩家是否被雪崩超越
bool Avalanche::isPlayerSurpassed(qreal playerX) const
{
    return distanceToPlayerLeft(playerX) < 10; // 距离小于10判定为被超越
}

// 计算雪崩前沿与玩家的距离
qreal Avalanche::distanceToPlayer(qreal playerX) const
{
    return qAbs(playerX - m_frontX); // 取绝对值
}

// 计算雪崩末端与玩家的距离
qreal Avalanche::distanceToPlayerLeft(qreal playerX) const
{
    qreal leftX = m_frontX - m_width;
    return playerX - leftX;
}

// 更新雪崩的形状（顶部曲线+右端半圆+底部曲线）
void Avalanche::updateShape(qreal playerX)
{
    const int points = 200;  // 顶部/底部分段数
    QPainterPath path;       // 路径对象

    qreal leftX = m_frontX - m_width; // 雪崩左端
    qreal rightX = m_frontX;          // 雪崩右端
    qreal step = (rightX - leftX) / (points - 1);   // 步长

    QVector<QPointF> topPoints, bottomPoints;
    // 生成顶部和底部点
    for (int i = 0; i < points; ++i) {
        qreal x = leftX + i * step;                       // 当前x坐标
        qreal y = m_terrain->getTerrainHeight(x);         // 地形高度
        qreal slope = m_terrain->getTerrainSlope(x);      // 地形斜率
        qreal topY = y - 60 - slope * 40;                 // 顶部点y坐标（高于地形）
        topPoints.append(QPointF(x, topY));               // 存储顶部点
        bottomPoints.append(QPointF(x, y));               // 存储底部点
    }

    // 右端半圆参数
    qreal groundY = bottomPoints.last().y(); // 地面y
    qreal topY = topPoints.last().y();       // 顶部y
    qreal radius = groundY - topY;           // 半圆半径
    QPointF arcCenter(rightX, groundY);      // 半圆圆心

    QVector<QPointF> arcPoints;
    const int arcSegments = 40;              // 半圆分段数
    // 生成右端半圆上的点（从上到下，顺时针270°到90°）
    for (int i = 0; i <= arcSegments; ++i) {
        qreal theta = M_PI * 1.5 + M_PI * (i / (qreal)arcSegments); // 角度
        qreal x = arcCenter.x() + radius * qCos(theta);             // 半圆x
        qreal y = arcCenter.y() + radius * qSin(theta);             // 半圆y
        arcPoints.append(QPointF(x, y));                            // 存储半圆点
    }

    // 构造雪崩封闭路径
    path.moveTo(topPoints.first());                 // 从顶部第一个点开始
    for (const auto& pt : topPoints) path.lineTo(pt);         // 顶部曲线
    for (const auto& pt : arcPoints) path.lineTo(pt);         // 右端半圆
    for (int i = bottomPoints.size() - 1; i >= 0; --i)
        path.lineTo(bottomPoints[i]);                         // 底部曲线
    path.closeSubpath();                                      // 闭合路径

    setPath(path); // 应用路径
}