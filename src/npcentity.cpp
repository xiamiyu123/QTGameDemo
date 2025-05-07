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
      m_movementDirection(0),
      m_npcType(NPCType::Pedestrian),
      m_aiEnabled(true),
      m_aiUpdateTimer(0)
{
    // 设置实体类型
    setEntityType(EntityType::NPC);

    // NPC默认外观
    setBrush(QBrush(Qt::blue));
    setPen(QPen(Qt::black, 1));
}

void NPCEntity::setNPCType(NPCType type)
{
    m_npcType = type;

    // 根据NPC类型设置不同外观
    switch (type) {
        case NPCType::Enemy:
            setBrush(QBrush(Qt::darkRed));
        break;
        case NPCType::Friendly:
            setBrush(QBrush(Qt::green));
        break;
        case NPCType::Pedestrian:
            default:
                setBrush(QBrush(Qt::blue));
        break;
    }
}

void NPCEntity::setMovementDirection(int direction)
{
    // 限制方向值为 -1、0、1
    m_movementDirection = qBound(-1, direction, 1);
}

qreal NPCEntity::getTargetVelocityX() const
{
    // 返回基于方向和速度的目标速度
    return m_movementDirection * m_movementSpeed;
}

void NPCEntity::updateSpecialAbility(float deltaTime)
{
    // 基本AI行为 - 周期性改变方向
    if (!m_aiEnabled) return;

    m_aiUpdateTimer += deltaTime;

    // 每2-5秒随机改变方向
    if (m_aiUpdateTimer >= 3.0) {
        m_aiUpdateTimer = 0;

        // 随机选择新方向
        int newDirection = QRandomGenerator::global()->bounded(3) - 1; // -1, 0, 1
        setMovementDirection(newDirection);
    }
}