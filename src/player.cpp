#include "player.h"
#include <QBrush>
#include <QPen>
#include <QGraphicsScene>
#include <QDebug>

Player::Player(QGraphicsItem *parent)
    : QGraphicsRectItem(0, 0, 30, 30, parent),
      m_velocityX(0),
      m_velocityY(0),
      m_movingLeft(false),
      m_movingRight(false),
      m_onGround(false)
{
    // 设置玩家外观
    setBrush(QBrush(Qt::red));
    setPen(QPen(Qt::black, 2));

    // 允许接收键盘焦点
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFocus();

    // 创建物理组件
    m_physicsComponent = new PhysicsComponent(this);
    m_physicsComponent->setGravity(1000);
    m_physicsComponent->setJumpForce(-500);

    // 注册到物理系统
    PhysicsSystem::instance().registerObject(this);
}

Player::~Player()
{
    // 从物理系统中移除
    PhysicsSystem::instance().unregisterObject(this);
    delete m_physicsComponent;
}

void Player::move(qreal dx, qreal dy)
{
    setPos(x() + dx, y() + dy);
}

void Player::update()
{
    // 处理键盘输入导致的水平移动
    if (m_movingLeft) {
        m_velocityX = -5;
    } else if (m_movingRight) {
        m_velocityX = 5;
    } else {
        m_velocityX = 0;
    }

    // 只更新水平移动，垂直移动由物理系统管理
    move(m_velocityX, 0);
}

void Player::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()) {
    case Qt::Key_Left:
        if (event->isAutoRepeat())
            return; // 忽略自动重复事件
        m_movingLeft = true;
        qDebug() << "Left key pressed";
        break;
    case Qt::Key_Right:
        if (event->isAutoRepeat())
            return; // 忽略自动重复事件
        m_movingRight = true;
        qDebug() << "Right key pressed";
        break;
    case Qt::Key_Space:
    case Qt::Key_Up:
        if (event->isAutoRepeat())
            return;
        // 触发跳跃
        m_physicsComponent->jump();
        qDebug() << "Jump";
        break;
    default:
        return;
    }
}

void Player::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key()) {
    case Qt::Key_Left:
        if (event->isAutoRepeat())
            return; // 忽略自动重复事件
        m_movingLeft = false;
        qDebug() << "Left key released";
        break;
    case Qt::Key_Right:
        if (event->isAutoRepeat())
            return; // 忽略自动重复事件
        m_movingRight = false;
        qDebug() << "Right key released";
        break;
    default:
        return;
    }
}

// 实现IPhysicsObject接口
QPointF Player::position() const
{
    return pos();
}

void Player::setPosition(const QPointF& pos)
{
    setPos(pos);
}

QPointF Player::velocity() const
{
    return QPointF(m_velocityX, m_velocityY);
}

void Player::setVelocity(const QPointF& velocity)
{
    m_velocityX = velocity.x();
    m_velocityY = velocity.y();
}

QRectF Player::boundingRect() const
{
    return QGraphicsRectItem::boundingRect();
}

bool Player::isOnGround() const
{
    return m_onGround;
}

void Player::setOnGround(bool onGround)
{
    m_onGround = onGround;
}

void Player::updatePhysics(float deltaTime) {
    // 应用水平移动，使用更大的速度值
    if (m_movingLeft) {
        m_velocityX = -300;  // 从-5增加到-300
    } else if (m_movingRight) {
        m_velocityX = 300;   // 从5增加到300
    } else {
        m_velocityX = 0;
    }
    // 更新水平位置
    setX(x() + m_velocityX * deltaTime);

    // 应用重力（垂直方向物理）
    m_physicsComponent->applyGravity(deltaTime);
}