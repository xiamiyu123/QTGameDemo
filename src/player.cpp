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
      m_onGround(false),
      m_moveSpeed(300),
      m_slopeSlideSpeed(0)
{
    // 设置物理组件
    m_physicsComponent = new PhysicsComponent(this);
    m_physicsComponent->setGravity(1000);
    m_physicsComponent->setJumpForce(-500);
    m_physicsComponent->setFrictionFactor(0.85); // 设置摩擦力
    // 设置玩家外观
    setBrush(QBrush(Qt::red));
    setPen(QPen(Qt::black, 2));

    // 允许接收键盘焦点
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFocus();


    // 注册到物理系统
    PhysicsSystem::instance().registerObject(this);

}

Player::~Player()
{
    // 从物理系统中移除
    PhysicsSystem::instance().unregisterObject(this);
    delete m_physicsComponent;
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
    // 使用物理组件处理移动和重力
    m_physicsComponent->applyHorizontalMovement(deltaTime);
    m_physicsComponent->applyGravity(deltaTime);
}

qreal Player::getSlopeSlideSpeed() const { return m_slopeSlideSpeed; }
void Player::setSlopeSlideSpeed(qreal speed) { m_slopeSlideSpeed = speed; }
qreal Player::getMoveSpeed() const { return m_moveSpeed; }
void Player::setMoveSpeed(qreal speed) { m_moveSpeed = speed; }
bool Player::isMovingLeft() const { return m_movingLeft; }
bool Player::isMovingRight() const { return m_movingRight; }