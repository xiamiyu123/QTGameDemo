//
// Created by xiami on 2025/5/29.
//
// flyingnpc.cpp
#include "flyingnpc.h"
#include <QBrush>
#include <QPen>

FlyingNPC::FlyingNPC(QGraphicsItem *parent)
    : NPCEntity(NPCType::Flying, 25, 20, parent),
      m_animationTimer(0)
{
}

FlyingNPC::FlyingNPC(qreal width, qreal height, QGraphicsItem *parent)
    : NPCEntity(NPCType::Flying, width, height, parent),
      m_animationTimer(0)
{
}

void FlyingNPC::initializeNPC()
{
    // 飞鸟配置
    setMovementSpeed(120);  // 较快的移动速度
    setBrush(QBrush(Qt::cyan));
    setPen(QPen(Qt::blue, 2));
    
    // 重置计时器
    m_animationTimer = 0;
}

void FlyingNPC::resetNPC()
{
    // 调用基类重置
    NPCEntity::resetNPC();
    
    // 重置飞鸟特有状态
    m_animationTimer = 0;
}

void FlyingNPC::updateSpecialAbility(float deltaTime)
{
    // 飞鸟特殊能力：不受重力影响，保持高度
    // 设置垂直速度为0，保持当前高度
    QPointF currentVel = velocity();
    setVelocity(QPointF(currentVel.x(), 0));
}

void FlyingNPC::updateAI(float deltaTime)
{
    // 更新AI计时器
    m_aiUpdateTimer += deltaTime;
    
    // 简单的AI：继续向右飞行
    // 移动逻辑已在getTargetVelocityX()中处理
}

void FlyingNPC::updateAppearance(float deltaTime)
{
    // 简单的动画：改变颜色模拟飞行
    m_animationTimer += deltaTime;
    
    if (m_animationTimer > 0.3f) {
        // 每0.3秒切换颜色
        QColor currentColor = brush().color();
        if (currentColor == Qt::cyan) {
            setBrush(QBrush(Qt::lightGray));
        } else {
            setBrush(QBrush(Qt::cyan));
        }
        m_animationTimer = 0;
    }
}
