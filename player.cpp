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
      m_movingRight(false)
{
    // 设置玩家外观
    setBrush(QBrush(Qt::red));
    setPen(QPen(Qt::black, 2));
    
    // 允许接收键盘焦点
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFocus();
}

void Player::move(qreal dx, qreal dy)
{
    setPos(x() + dx, y() + dy);
}

void Player::update()
{
    // 处理键盘输入导致的移动
    if (m_movingLeft) {
        m_velocityX = -5;
    } else if (m_movingRight) {
        m_velocityX = 5;
    } else {
        m_velocityX = 0;
    }
    
    // 应用移动
    move(m_velocityX, 0);
}

void Player::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()) {
    case Qt::Key_Left:
        m_movingLeft = true;
        break;
    case Qt::Key_Right:
        m_movingRight = true;
        break;
    default:
        QGraphicsRectItem::keyPressEvent(event);
    }
}

void Player::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key()) {
    case Qt::Key_Left:
        m_movingLeft = false;
        break;
    case Qt::Key_Right:
        m_movingRight = false;
        break;
    default:
        QGraphicsRectItem::keyReleaseEvent(event);
    }
}