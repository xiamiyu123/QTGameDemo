#include "player.h"
#include <QBrush>
#include <QPen>
#include <QDebug>

Player::Player(QGraphicsItem *parent)
    : BasePhysicsEntity(30, 30, parent),
      m_movingLeft(false),
      m_movingRight(false),
      m_moveSpeed(500),
      m_jumpForce(-300)
{
    // 设置玩家外观
    setBrush(QBrush(Qt::red));
    setPen(QPen(Qt::black, 2));

    // 设置实体类型
    setEntityType(EntityType::Player);

    // 允许接收键盘焦点
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFocus();
}

Player::~Player()
{
    // 父类析构函数会处理注销和组件删除
}

void Player::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        return;

    switch(event->key()) {
        case Qt::Key_Left:
            m_movingLeft = true;
        qDebug() << "Left key pressed";
        break;
        case Qt::Key_Right:
            m_movingRight = true;
        qDebug() << "Right key pressed";
        break;
        case Qt::Key_Space:
        case Qt::Key_Up:
            jump();

        break;
    }
}

void Player::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        return;

    switch(event->key()) {
        case Qt::Key_Left:
            m_movingLeft = false;
        qDebug() << "Left key released";
        break;
        case Qt::Key_Right:
            m_movingRight = false;
        qDebug() << "Right key released";
        break;
    }
}

void Player::jump()
{
    if (isOnGround()) {
        QPointF vel = velocity();
        vel.setY(m_jumpForce);
        setVelocity(vel);
        setOnGround(false);
        qDebug() << "Jump";
    }
    else {
        qDebug() << "Jump is not available";
    }
}

qreal Player::getTargetVelocityX() const
{
    qreal targetVelocity = 0;

    if (m_movingLeft) {
        targetVelocity -= m_moveSpeed;
    }

    if (m_movingRight) {
        targetVelocity += m_moveSpeed;
    }

    return targetVelocity;
}