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
    // NPC类型枚举
    enum class NPCType {
        Pedestrian,  // 普通行人
        Enemy,       // 敌人
        Friendly     // 友好NPC
    };

    NPCEntity(qreal width = 25, qreal height = 25, QGraphicsItem *parent = nullptr);
    virtual ~NPCEntity() = default;

    // NPC特有属性
    void setNPCType(NPCType type);
    NPCType getNPCType() const { return m_npcType; }

    // AI行为控制
    void setMovementSpeed(qreal speed) { m_movementSpeed = speed; }
    void setAIEnabled(bool enabled) { m_aiEnabled = enabled; }
    bool isAIEnabled() const { return m_aiEnabled; }

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
    NPCType m_npcType;
    bool m_aiEnabled;
    qreal m_aiUpdateTimer;
};