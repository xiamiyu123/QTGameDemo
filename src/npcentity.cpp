//
// Created by xiami on 2025/5/7.
//
// npcentity.cpp
#include "npcentity.h"
#include <QBrush>
#include <QPen>
#include <QRandomGenerator>

NPCEntity::NPCEntity(qreal width, qreal height, QGraphicsItem *parent)
    : BasePhysicsEntity(width, height, parent),
      m_movementSpeed(100),
      m_movementDirection(1),
      m_aiUpdateTimer(0)
{
    // 设置实体类型
    setEntityType(EntityType::NPC);

    // NPC默认外观
    setBrush(QBrush(Qt::blue));
    setPen(QPen(Qt::black, 1));
}


void NPCEntity::setMovementDirection(int direction)
{
    // 限制方向值为 -1、0、1
    // -1表示向左移动，0表示停止，1表示向右移动
    m_movementDirection = qBound(-1, direction, 1);
}

qreal NPCEntity::getTargetVelocityX() const
{
    // 返回基于方向和速度的目标速度
    return m_movementDirection * m_movementSpeed;
}

void NPCEntity::updateSpecialAbility(float deltaTime)
{
    
}