// npcentity.h
#pragma once
#include "basephysicsentity.h"

/**
 * NPCEntity: NPC基类
 * 为游戏中的NPC提供基础实现，支持AI行为
 */
class NPCEntity : public BasePhysicsEntity
{
    Q_OBJECT

public:
    NPCEntity(qreal width = 25, qreal height = 25, QGraphicsItem *parent = nullptr);
    virtual ~NPCEntity() = default;

    // AI行为控制
    void setMovementSpeed(qreal speed) { m_movementSpeed = speed; }

    // 设置移动方向 (-1左, 0停, 1右)
    void setMovementDirection(int direction);

protected:
    // 获取AI控制的目标速度
    qreal getTargetVelocityX() const override;

    // NPC特性
    virtual void updateSpecialAbility(float deltaTime);

private:
    qreal m_movementSpeed;
    int m_movementDirection;  // -1左, 0停, 1右
    qreal m_aiUpdateTimer;
};