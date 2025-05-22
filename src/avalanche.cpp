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
      m_maxSpeed(650),      // 最大速度（比玩家快）
      m_width(6000),        // 雪崩宽度
      m_visibleRange(2000)  // 默认视距范围
{
    setZValue(-1); // 图层放在地形下方
    setBrush(QBrush(QColorConstants::Svg::white)); // 设置雪崩颜色为白色
    setPen(QPen(QColorConstants::Svg::white));     // 设置雪崩边框颜色为白色
}

void Avalanche::updateAvalanche(qreal deltaTime, qreal playerX)
{
    m_speed += m_acceleration * deltaTime;      // 速度随时间增加
    if (m_speed > m_maxSpeed) m_speed = m_maxSpeed; // 限制最大速度

    // 检查雪崩是否在玩家视野范围内
    if (m_frontX + m_width < playerX - m_visibleRange) {
        // 完全在视野左侧外，隐藏
        setVisible(false);
        return;
    } else {
        setVisible(true);
    }

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
    return distanceToPlayer(playerX) < 10; // 距离小于10判定为被超越
}

// 计算雪崩前沿与玩家的距离
qreal Avalanche::distanceToPlayer(qreal playerX) const
{
    return qAbs(playerX - m_frontX); // 取绝对值
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
    // 生成顶部和底部点
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
        path.lineTo(bottomPoints[i]);                         // 底部曲线
    path.closeSubpath();                                      // 闭合路径

    setPath(path); // 应用路径
}

// 添加线程安全方法实现

void Avalanche::updateAvalancheThreadSafe(qreal elapsed, qreal playerX)
{
    // 更新雪崩速度
    qreal localSpeed = m_speed + m_acceleration * elapsed;
    if (localSpeed > m_maxSpeed) localSpeed = m_maxSpeed;
    
    // 更新雪崩前沿位置 - 这是丢失的关键逻辑
    qreal localFrontX = m_frontX + localSpeed * elapsed;
    
    // 计算绘制数据 - 参考原始updateShape逻辑
    QVector<QPointF> topPoints, bottomPoints;
    QVector<QPointF> arcPoints;
    QPainterPath localPath;
    
    // 计算范围
    qreal leftX = localFrontX - m_width;
    qreal rightX = localFrontX;
    
    // 只计算可见部分
    qreal leftLimit = qMax(leftX, playerX - m_visibleRange * 1.2);
    qreal rightLimit = qMin(rightX, playerX + m_visibleRange * 0.5);
    
    // 存储可见性状态
    bool isVisible = !(localFrontX - m_width > playerX + m_visibleRange || 
                       localFrontX < playerX - m_visibleRange * 1.2);
    
    // 如果可见部分存在，计算绘制路径
    if (leftLimit < rightLimit && isVisible) {
        // 动态计算点数，基于可见宽度
        qreal visibleWidth = rightLimit - leftLimit;
        int points = qMin(200, qMax(50, int(visibleWidth / 30)));
        qreal step = visibleWidth / (points - 1);
        
        // 生成顶部和底部点
        for (int i = 0; i < points; ++i) {
            qreal x = leftLimit + i * step;
            qreal y = m_terrain->getTerrainHeight(x);
            qreal slope = m_terrain->getTerrainSlope(x);
            qreal topY = y - 60 - slope * 40;
            topPoints.append(QPointF(x, topY));
            bottomPoints.append(QPointF(x, y));
        }
        
        // 只在雪崩前沿位于可见区域时绘制半圆
        bool drawArc = (rightLimit > localFrontX - 10);
        if (drawArc) {
            // 右端半圆参数
            qreal groundY = m_terrain->getTerrainHeight(localFrontX);
            qreal topY = groundY - 60 - m_terrain->getTerrainSlope(localFrontX) * 40;
            qreal radius = groundY - topY; // 顶部到地面的距离
            QPointF arcCenter(localFrontX, groundY);
            
            const int arcSegments = 40; // 恢复原来的40段
            for (int i = 0; i <= arcSegments; ++i) {
                // 角度从270°到90°（顺时针，从上到下）
                qreal theta = M_PI * 1.5 + M_PI * (i / (qreal)arcSegments);
                qreal x = arcCenter.x() + radius * qCos(theta);
                qreal y = arcCenter.y() + radius * qSin(theta);
                arcPoints.append(QPointF(x, y));
            }
        }
    }
    
    // 将结果存储到线程安全的缓冲区
    QMutexLocker locker(&m_mutex);
    m_threadCalculatedFrontX = localFrontX;    // 存储计算的前沿位置
    m_threadCalculatedSpeed = localSpeed;      // 存储计算的速度
    m_threadCalculatedTopPoints = topPoints;   // 存储顶部点
    m_threadCalculatedBottomPoints = bottomPoints; // 存储底部点
    m_threadCalculatedArcPoints = arcPoints;   // 存储半圆点
    m_threadCalculatedVisible = isVisible;     // 存储可见性
    m_hasThreadResults = true;
}

void Avalanche::applyThreadResults()
{
    QMutexLocker locker(&m_mutex);
    if (m_hasThreadResults) {
        // 应用线程计算的结果到实际状态
        m_frontX = m_threadCalculatedFrontX;  // 更新前沿位置
        m_speed = m_threadCalculatedSpeed;    // 更新速度
        
        // 设置可见性
        setVisible(m_threadCalculatedVisible);
        
        // 如果不可见，直接返回
        if (!m_threadCalculatedVisible) {
            m_hasThreadResults = false;
            return;
        }
        
        // 构造路径
        QPainterPath path;
        if (!m_threadCalculatedTopPoints.isEmpty()) {
            path.moveTo(m_threadCalculatedTopPoints.first());
            
            // 添加顶部点
            for (const auto& pt : m_threadCalculatedTopPoints) {
                path.lineTo(pt);
            }
            
            // 添加半圆点（如果有）
            for (const auto& pt : m_threadCalculatedArcPoints) {
                path.lineTo(pt);
            }
            
            // 如果没有半圆点且有顶部点和底部点，添加连接线
            if (m_threadCalculatedArcPoints.isEmpty() && 
                !m_threadCalculatedTopPoints.isEmpty() && 
                !m_threadCalculatedBottomPoints.isEmpty()) {
                path.lineTo(m_threadCalculatedTopPoints.last().x(), 
                           m_threadCalculatedBottomPoints.last().y());
            }
            
            // 添加底部点（反向）
            for (int i = m_threadCalculatedBottomPoints.size() - 1; i >= 0; --i) {
                path.lineTo(m_threadCalculatedBottomPoints[i]);
            }
            
            path.closeSubpath();
        }
        
        // 应用路径到图形项
        setPath(path);
        m_hasThreadResults = false;
    }
}
