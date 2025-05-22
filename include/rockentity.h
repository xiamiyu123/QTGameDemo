#pragma once
#include "basephysicsentity.h"
#include <QBrush>
#include <QPen>

class RockEntity : public BasePhysicsEntity
{
    Q_OBJECT
public:
    RockEntity(qreal width = 30, qreal height = 30, QGraphicsItem* parent = nullptr)
        : BasePhysicsEntity(width, height, parent)
    {
        setZValue(-2);
        setBrush(QBrush(Qt::black));
        setPen(QPen(Qt::black, 2));
        setEntityType(EntityType::Obstacle);
        // 固定石头，不受物理影响
        m_physicsComponent->setGravity(0);
        m_physicsComponent->setFrictionFactor(0);
    }
    ~RockEntity() override = default;

    // 禁止物理系统更新石头位置
    void updatePhysics(float) override {}
};