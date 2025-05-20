//
// Created by xiami on 2025/5/7.
//
// basephysicsentity.cpp
#include "basephysicsentity.h"
#include <QBrush>
#include <QPen>

BasePhysicsEntity::BasePhysicsEntity(qreal width, qreal height, QGraphicsItem *parent)
    : QGraphicsRectItem(0, 0, width, height, parent),
    m_velocityX(0),
    m_velocityY(0),
    m_onGround(false),
    m_slopeSlideSpeed(0),
    m_entityType(EntityType::Obstacle),
    rotation(0)
{
    // 设置物理组件
    m_physicsComponent = new PhysicsComponent(this);
    // 默认物理参数
    m_physicsComponent->setGravity(600);
    m_physicsComponent->setFrictionFactor(0.85);

    // 默认外观
    setBrush(QBrush(Qt::gray));
    setPen(QPen(Qt::black, 1));

    // 注册到物理系统
    PhysicsSystem::instance().registerObject(this);
}

BasePhysicsEntity::~BasePhysicsEntity()
{
    // 从物理系统中移除
    PhysicsSystem::instance().unregisterObject(this);
    delete m_physicsComponent;
}

QPointF BasePhysicsEntity::position() const
{
    return pos();
}

void BasePhysicsEntity::setPosition(const QPointF& pos)
{
    setPos(pos);
}

QPointF BasePhysicsEntity::velocity() const
{
    return QPointF(m_velocityX, m_velocityY);
}

void BasePhysicsEntity::setVelocity(const QPointF& velocity)
{
    m_velocityX = velocity.x();
    m_velocityY = velocity.y();
}

QRectF BasePhysicsEntity::boundingRect() const
{
    return QGraphicsRectItem::boundingRect();
}

bool BasePhysicsEntity::isOnGround() const
{
    return m_onGround;
}

void BasePhysicsEntity::setOnGround(bool onGround)
{
    if (m_onGround == onGround) return; // 状态未改变
    m_onGround = onGround;

}

void BasePhysicsEntity::updatePhysics(float deltaTime)
{
    // 使用物理组件处理基本物理
    m_physicsComponent->applyHorizontalMovement(deltaTime, getTargetVelocityX());
    m_physicsComponent->applyGravity(deltaTime);
}

qreal BasePhysicsEntity::getSlopeSlideSpeed() const
{
    return m_slopeSlideSpeed;
}

void BasePhysicsEntity::setSlopeSlideSpeed(qreal speed)
{
    m_slopeSlideSpeed = speed;
}

void BasePhysicsEntity::setRotation(qreal angle)
{
    rotation = angle;
    QGraphicsRectItem::setRotation(angle);
}
