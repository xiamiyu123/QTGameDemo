// 包含物理系统接口和实现的头文件
#pragma once

#include <QPointF>
#include <QRectF>
#include <QtPlugin>

/*
 * IPhysicsObject: 对外提供物理对象必须实现的纯虚接口。
 * 负责获取/设置位置、速度、边界框，并处理是否在地面上等状态，以及物理更新接口。
 */
// 在 physical.h 中修改 IPhysicsObject 接口
class IPhysicsObject {
public:
    virtual ~IPhysicsObject() = default;

    // 现有方法保持不变
    virtual QPointF position() const = 0;
    virtual void setPosition(const QPointF& pos) = 0;
    virtual QPointF velocity() const = 0;
    virtual void setVelocity(const QPointF& velocity) = 0;
    virtual QRectF boundingRect() const = 0;
    virtual bool isOnGround() const = 0;
    virtual void setOnGround(bool onGround) = 0;
    virtual void updatePhysics(float deltaTime) = 0;

    // 新增斜坡滑行相关方法
    virtual qreal getSlopeSlideSpeed() const = 0;
    virtual void setSlopeSlideSpeed(qreal speed) = 0;
    virtual qreal getMoveSpeed() const = 0;
    virtual void setMoveSpeed(qreal speed) = 0;
    virtual bool isMovingLeft() const = 0;
    virtual bool isMovingRight() const = 0;

};

/*
 * PhysicsComponent: 附加在 IPhysicsObject 上的组件，负责执行重力、跳跃等物理操作
 */
class PhysicsComponent {
public:

    void setGravity(qreal gravity) { m_gravity = gravity; }
    void setJumpForce(qreal force) { m_jumpForce = force; }
    qreal getGravity() const { return m_gravity; }
    qreal getJumpForce() const { return m_jumpForce; }

    void setFrictionFactor(qreal factor) { m_frictionFactor = qBound(0.0, factor, 1.0); }
    qreal getFrictionFactor() const { return m_frictionFactor; }

    void applyGravity(float deltaTime) {
        QPointF vel = m_owner->velocity();
        vel.setY(vel.y() + m_gravity * deltaTime);
        m_owner->setVelocity(vel);

        QPointF pos = m_owner->position();
        pos.setY(pos.y() + vel.y() * deltaTime);
        m_owner->setPosition(pos);
    }

    void jump() {
        if (m_owner->isOnGround()) {
            QPointF vel = m_owner->velocity();
            vel.setY(m_jumpForce);
            m_owner->setVelocity(vel);
            m_owner->setOnGround(false);
        }
    }

    // 新增处理水平移动和斜坡滑行的方法
void applyHorizontalMovement(float deltaTime) {
    qreal currentVelocityX = m_owner->velocity().x();
    qreal targetVelocityX = 0;
    qreal acceleration = 0;

    // 确定目标速度
    if (m_owner->isMovingLeft()) {
        targetVelocityX = -m_owner->getMoveSpeed();
    } else if (m_owner->isMovingRight()) {
        targetVelocityX = m_owner->getMoveSpeed();
    }

    // 添加斜坡滑行速度
    targetVelocityX += m_owner->getSlopeSlideSpeed();

    // 根据是否在地面上调整加速率
    qreal airControlFactor = m_owner->isOnGround() ? 1.0 : 0.3;

    // 计算需要应用的加速度
    if (targetVelocityX != 0) {
        // 加速或保持速度
        if ((targetVelocityX > 0 && currentVelocityX < targetVelocityX) ||
            (targetVelocityX < 0 && currentVelocityX > targetVelocityX)) {

            // 检测是否在转向（当前速度与目标速度方向相反）
            if ((currentVelocityX > 0 && targetVelocityX < 0) ||
                (currentVelocityX < 0 && targetVelocityX > 0)) {
                acceleration = m_accelerationRate * m_directionChangeMultiplier * airControlFactor;
            } else {
                acceleration = m_accelerationRate * airControlFactor;
            }
        }
    } else if (currentVelocityX != 0) {
        // 减速到停止
        acceleration = m_decelerationRate * airControlFactor;
    }

    // 应用加速度
    if (acceleration > 0) {
        if (targetVelocityX > currentVelocityX) {
            currentVelocityX = qMin(currentVelocityX + acceleration * deltaTime, targetVelocityX);
        } else if (targetVelocityX < currentVelocityX) {
            currentVelocityX = qMax(currentVelocityX - acceleration * deltaTime, targetVelocityX);
        } else if (targetVelocityX == 0) {
            // 减速到停止
            if (currentVelocityX > 0) {
                currentVelocityX = qMax(currentVelocityX - acceleration * deltaTime, 0.0);
            } else {
                currentVelocityX = qMin(currentVelocityX + acceleration * deltaTime, 0.0);
            }
        }
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
    qreal m_accelerationRate;    // 加速率
    qreal m_decelerationRate;    // 减速率
    qreal m_directionChangeMultiplier; // 转向时的加速系数

public:
    explicit PhysicsComponent(IPhysicsObject* owner)
        : m_owner(owner), m_gravity(50), m_jumpForce(-15),
          m_maxFallSpeed(200), m_frictionFactor(0.92),
          m_accelerationRate(800), m_decelerationRate(1200),
          m_directionChangeMultiplier(1.5) {}

    // 设置加速率
    void setAccelerationRate(qreal rate) { m_accelerationRate = rate; }
    void setDecelerationRate(qreal rate) { m_decelerationRate = rate; }

private:
    IPhysicsObject* m_owner;
    qreal m_gravity;
    qreal m_jumpForce;
    qreal m_maxFallSpeed;
    qreal m_frictionFactor;  // 控制速度平滑变化的因子
};


/*
 * PhysicsSystem: 单例模式管理所有注册的 IPhysicsObject，并在每帧调用其 updatePhysics
 */
class PhysicsSystem {
public:
    // 获取全局单例实例
    static PhysicsSystem& instance() {
        static PhysicsSystem instance;
        return instance;
    }

    // 注册需要物理更新的对象
    void registerObject(IPhysicsObject* obj) {
        if (!m_physicsObjects.contains(obj))
            m_physicsObjects.append(obj);
    }

    // 注销对象，不再进行物理更新
    void unregisterObject(IPhysicsObject* obj) {
        m_physicsObjects.removeAll(obj);
    }

    /*
     * update: 对所有已注册对象调用 updatePhysics，
     * deltaTime 为距离上次调用的时间间隔（秒）
     */
    void update(float deltaTime) {
        for (auto* obj : m_physicsObjects) {
            obj->updatePhysics(deltaTime);
        }
    }

private:
    QList<IPhysicsObject*> m_physicsObjects;  // 存储所有已注册物理对象的列表
};