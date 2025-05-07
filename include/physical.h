// physical.h - 简化的物理系统接口
#pragma once

#include <QPointF>
#include <QRectF>
#include <QtPlugin>
#include <QList>
#include <QObject>

/**
 * IPhysicsObject: 物理对象接口类
 * 定义所有需要物理行为的游戏对象必须实现的纯虚接口
 */
class IPhysicsObject {
public:
    virtual ~IPhysicsObject() = default;

    // 基础属性访问
    virtual QPointF position() const = 0;
    virtual void setPosition(const QPointF &pos) = 0;
    virtual QPointF velocity() const = 0;
    virtual void setVelocity(const QPointF &velocity) = 0;
    virtual QRectF boundingRect() const = 0;

    // 地面状态
    virtual bool isOnGround() const = 0;
    virtual void setOnGround(bool onGround) = 0;

    // 物理更新
    virtual void updatePhysics(float deltaTime) = 0;

    // 斜坡相关
    virtual qreal getSlopeSlideSpeed() const = 0;
    virtual void setSlopeSlideSpeed(qreal speed) = 0;

    // 实体类别 - 用于碰撞过滤
    enum class EntityType {
        Player,
        NPC,
        Obstacle,
        Collectible
    };

    virtual EntityType getEntityType() const = 0;
};

/**
 * 简化的物理组件 - 只负责基础物理行为
 */
class PhysicsComponent {
public:
    explicit PhysicsComponent(IPhysicsObject *owner)
        : m_owner(owner), m_gravity(600),
          m_frictionFactor(0.85),
          m_accelerationRate(800), m_decelerationRate(1200),
          m_airControlFactor(0.3) {
    }

    // 简化的设置/获取方法
    void setGravity(qreal gravity) { m_gravity = gravity; }
    void setFrictionFactor(qreal factor) { m_frictionFactor = qBound(0.0, factor, 1.0); }
    void setAccelerationRate(qreal rate) { m_accelerationRate = rate; }
    void setAirControlFactor(qreal factor) { m_airControlFactor = factor; }

    qreal getGravity() const { return m_gravity; }
    qreal getFrictionFactor() const { return m_frictionFactor; }
    qreal getAccelerationRate() const { return m_accelerationRate; }

    /**
     * 应用重力
     */
    void applyGravity(float deltaTime) {
        QPointF vel = m_owner->velocity();
        vel.setY(vel.y() + m_gravity * deltaTime);
        m_owner->setVelocity(vel);

        QPointF pos = m_owner->position();
        pos.setY(pos.y() + vel.y() * deltaTime);
        m_owner->setPosition(pos);
    }

    /**
     * 应用水平移动 - 简化版
     */
    void applyHorizontalMovement(float deltaTime, qreal targetVelocityX) {
        qreal currentVelocityX = m_owner->velocity().x();
        qreal acceleration;
        qreal controlFactor = m_owner->isOnGround() ? 1.0 : m_airControlFactor;

        // 添加斜坡速度到目标速度
        targetVelocityX += m_owner->getSlopeSlideSpeed();

        // 确定加速度
        if (targetVelocityX != 0) {
            // 检测是否在转向
            if ((currentVelocityX > 0 && targetVelocityX < 0) ||
                (currentVelocityX < 0 && targetVelocityX > 0)) {
                acceleration = m_decelerationRate * controlFactor;
            } else {
                acceleration = m_accelerationRate * controlFactor;
            }
        } else if (currentVelocityX != 0) {
            // 减速停止
            acceleration = m_decelerationRate * controlFactor;
        } else {
            return; // 静止状态
        }

        // 应用加速度
        if (targetVelocityX > currentVelocityX) {
            currentVelocityX = qMin(currentVelocityX + acceleration * deltaTime, targetVelocityX);
        } else if (targetVelocityX < currentVelocityX) {
            currentVelocityX = qMax(currentVelocityX - acceleration * deltaTime, targetVelocityX);
        }

        // 应用摩擦力
        if (m_owner->isOnGround() && qAbs(targetVelocityX) < 0.1) {
            currentVelocityX *= m_frictionFactor;
        }

        // 更新速度和位置
        QPointF vel = m_owner->velocity();
        vel.setX(currentVelocityX);
        m_owner->setVelocity(vel);

        QPointF pos = m_owner->position();
        pos.setX(pos.x() + vel.x() * deltaTime);
        m_owner->setPosition(pos);
    }

private:
    IPhysicsObject *m_owner;
    qreal m_gravity;
    qreal m_frictionFactor;
    qreal m_accelerationRate;
    qreal m_decelerationRate;
    qreal m_airControlFactor;
};

/**
 * 物理系统 - 管理所有物理对象并处理碰撞
 */
class PhysicsSystem {
public:
    static PhysicsSystem &instance() {
        static PhysicsSystem instance;
        return instance;
    }

    void registerObject(IPhysicsObject *obj) {
        if (!m_physicsObjects.contains(obj))
            m_physicsObjects.append(obj);
    }

    void unregisterObject(IPhysicsObject *obj) {
        m_physicsObjects.removeAll(obj);
    }

    void update(float deltaTime) {
        for (auto *obj : m_physicsObjects) {
            obj->updatePhysics(deltaTime);
        }
    }

    // 获取所有已注册的物理对象
    const QList<IPhysicsObject*>& getPhysicsObjects() const {
        return m_physicsObjects;
    }

private:
    QList<IPhysicsObject *> m_physicsObjects;
};