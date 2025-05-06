// 包含物理系统接口和实现的头文件
#pragma once

#include <QPointF>
#include <QRectF>
#include <QtPlugin>

/*
 * IPhysicsObject: 对外提供物理对象必须实现的纯虚接口。
 * 负责获取/设置位置、速度、边界框，并处理是否在地面上等状态，以及物理更新接口。
 */
class IPhysicsObject {
public:
    virtual ~IPhysicsObject() = default;

    // 获取当前对象的位置（x,y）
    virtual QPointF position() const = 0;
    // 设置对象的位置
    virtual void setPosition(const QPointF& pos) = 0;

    // 获取当前对象的速度（x,y）
    virtual QPointF velocity() const = 0;
    // 设置对象的速度
    virtual void setVelocity(const QPointF& velocity) = 0;

    // 获取对象的边界矩形（用于碰撞检测）
    virtual QRectF boundingRect() const = 0;

    // 判断对象是否与地面接触
    virtual bool isOnGround() const = 0;
    // 设置对象的在地面状态
    virtual void setOnGround(bool onGround) = 0;

    // 按照给定时间增量更新对象的物理状态
    virtual void updatePhysics(float deltaTime) = 0;
};

/*
 * PhysicsComponent: 附加在 IPhysicsObject 上的组件，负责执行重力、跳跃等物理操作
 */
class PhysicsComponent {
public:
    // 构造时关联所属对象
    PhysicsComponent(IPhysicsObject* owner) : m_owner(owner) {}

    // 设置重力加速度，正值向下
    void setGravity(qreal gravity) { m_gravity = gravity; }
    // 设置跳跃初始冲力，负值向上
    void setJumpForce(qreal force) { m_jumpForce = force; }
    qreal getGravity() const { return m_gravity; }
    qreal getJumpForce() const { return m_jumpForce; }

    /*
     * applyGravity: 根据 deltaTime 应用重力加速度
     * 1. 更新速度：v_y += g * dt
     * 2. 限制最大下落速度（若需要）
     * 3. 更新位置：y += v_y * dt
     */
    void applyGravity(float deltaTime) {
        // 获取当前速度
        QPointF vel = m_owner->velocity();


        // 在 y 方向增加重力加速度
        vel.setY(vel.y() + m_gravity * deltaTime);
        // TODO: 可在此添加对 m_maxFallSpeed 的限制

        // 设置新的速度
        m_owner->setVelocity(vel);

        // 按新速度更新位置
        QPointF pos = m_owner->position();
        pos.setY(pos.y() + vel.y() * deltaTime);
        m_owner->setPosition(pos);
    }

    /*
     * jump: 如果当前在地面状态，则赋予向上跳跃的初速度，并设置为非地面状态
     */
    void jump() {
        // 仅在物体接触地面时才允许跳跃，以防止空中二次跳跃
        if (m_owner->isOnGround()) {
            // 获取当前速度向量
            QPointF vel = m_owner->velocity();
            // 将垂直速度分量设置为跳跃初速度（m_jumpForce 为负值表示向上）
            vel.setY(m_jumpForce);
            // 应用新的速度，让物体产生向上的初始冲力
            m_owner->setVelocity(vel);

            // 将接触地面状态设为 false，
            // 标记物体已离地，以避免未落地前再次触发跳跃
            m_owner->setOnGround(false);
        }
    }

private:
    IPhysicsObject* m_owner;      // 所属物理对象
    qreal m_gravity = 50;        // 默认重力加速度
    qreal m_jumpForce = -15;      // 默认跳跃初速度
    qreal m_maxFallSpeed = 200;    // 最大下落速度（目前未使用）
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