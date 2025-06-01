//
// Created by xiami on 2025/5/7.
//
// basephysicsentity.h
#pragma once

#include <QObject>
#include <QGraphicsRectItem>
#include "physical.h"

/**
 * BasePhysicsEntity: 物理实体基类
 * 提供IPhysicsObject接口的默认实现，简化新实体创建
 */
class BasePhysicsEntity : public QObject, public QGraphicsRectItem, public IPhysicsObject
{
    Q_OBJECT

public:
    BasePhysicsEntity(qreal width, qreal height, QGraphicsItem *parent = nullptr);
    virtual ~BasePhysicsEntity();

    // IPhysicsObject接口实现
    QPointF position() const override;
    void setPosition(const QPointF& pos) override;
    QPointF velocity() const override;
    void setVelocity(const QPointF& velocity) override;
    QRectF boundingRect() const override;
    bool isOnGround() const override;
    void setOnGround(bool onGround) override;
    void updatePhysics(float deltaTime) override;
    qreal getSlopeSlideSpeed() const override;
    void setSlopeSlideSpeed(qreal speed) override;
    // 默认实体类型为Obstacle
    EntityType getEntityType() const override { return m_entityType; }
    void setEntityType(EntityType type) { m_entityType = type; }
    // 设置旋转角度
    void setRotation(qreal angle);    // 获取物理组件
    PhysicsComponent* getPhysicsComponent() { return m_physicsComponent; }
    qreal getRotation() const { return rotation; }

protected:
    qreal rotation; // 旋转角度(角度制)
    qreal m_velocityX;
    qreal m_velocityY;
    bool m_onGround;
    qreal m_slopeSlideSpeed;
    PhysicsComponent* m_physicsComponent;
    EntityType m_entityType;

    // 子类可以重写这个方法来提供目标速度
    virtual qreal getTargetVelocityX() const { return 0.0; }
};