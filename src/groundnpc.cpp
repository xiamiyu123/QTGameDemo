//
// Created by xiami on 2025/5/29.
//
// groundnpc.cpp
#include "groundnpc.h"
#include <QBrush>
#include <QPen>

GroundNPC::GroundNPC(QGraphicsItem *parent)
    : NPCEntity(NPCType::Ground, 30, 30, parent),
      m_animationTimer(0)
{
}

GroundNPC::GroundNPC(qreal width, qreal height, QGraphicsItem *parent)
    : NPCEntity(NPCType::Ground, width, height, parent),
      m_animationTimer(0)
{
}

void GroundNPC::initializeNPC()
{
    // 地面动物配置
    setMovementSpeed(80);  // 稍慢的移动速度
    setBrush(QBrush(Qt::darkGreen));
    setPen(QPen(Qt::black, 2));
    
    // 禁用重力 - 地面NPC应该始终贴地移动
    getPhysicsComponent()->setGravity(0);
    
    // 重置计时器
    m_animationTimer = 0;
}

void GroundNPC::resetNPC()
{
    // 调用基类重置
    NPCEntity::resetNPC();
    
    // 重置地面动物特有状态
    m_animationTimer = 0;
}

void GroundNPC::updateSpecialAbility(float deltaTime)
{
    // 地面动物特殊能力：简单的向右移动
    // 不需要特殊能力，只是向右移动
}

void GroundNPC::updateAI(float deltaTime)
{
    // 更新AI计时器
    m_aiUpdateTimer += deltaTime;
    
    // 简单的AI：继续向右移动
    // 移动逻辑已在getTargetVelocityX()中处理
}

void GroundNPC::updateAppearance(float deltaTime)
{
    // 简单的动画：改变颜色模拟跑动
    m_animationTimer += deltaTime;
    
    if (m_animationTimer > 0.5f) {
        // 每0.5秒切换颜色
        QColor currentColor = brush().color();
        if (currentColor == Qt::darkGreen) {
            setBrush(QBrush(Qt::green));
        } else {
            setBrush(QBrush(Qt::darkGreen));
        }
        m_animationTimer = 0;
    }
}
