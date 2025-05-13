#include "player.h"
#include <QBrush>
#include <QPen>
#include <QDebug>
#include <qpainter.h>

#include "terraingenerator.h"

Player::Player(QGraphicsItem *parent)
    : BasePhysicsEntity(30, 30, parent),
      is_fallen(false),
      keyLeft(false),
      keySpace(false),
      keyRight(false),
      m_moveSpeed(500),
      rotateSpeed(3),
      m_jumpForce(-300) {
    // 设置玩家外观
    setBrush(QBrush(Qt::red));
    setPen(QPen(Qt::black, 2));

    // 设置实体类型
    setEntityType(EntityType::Player);

    // 允许接收键盘焦点
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFocus();
}

Player::~Player() {
    // 父类析构函数会处理注销和组件删除
}

void Player::keyPressEvent(QKeyEvent *event) {
    if (event->isAutoRepeat())
        return;

    switch (event->key()) {
        case Qt::Key_Left:
            keyLeft = true;
            qDebug() << "Left key pressed";
            break;
        case Qt::Key_Right:
            keyRight = true;
            qDebug() << "Right key pressed";
            break;
        case Qt::Key_Space:
        case Qt::Key_Up:
            keySpace = true;
            qDebug() << "Space key pressed";
        // 处理跳跃
            jump();
            break;
    }
}

void Player::keyReleaseEvent(QKeyEvent *event) {
    if (event->isAutoRepeat())
        return;

    switch (event->key()) {
        case Qt::Key_Left:
            keyLeft = false;
            qDebug() << "Left key released";
            break;
        case Qt::Key_Right:
            keyRight = false;
            qDebug() << "Right key released";
            break;
        case Qt::Key_Space:
        case Qt::Key_Up:
            keySpace = false;
            qDebug() << "Space key released";
            break;
        case Qt::Key_T://显示调试信息
            qDebug() << "Rotation:" << rotation;
    }
}

void Player::jump() {
    if (isOnGround()) {
        QPointF vel = velocity();
        vel.setY(m_jumpForce);
        setVelocity(vel);
        setOnGround(false);
        qDebug() << "Jump";
    } else {
        //qDebug() << "Jump is not available";
    }
}

qreal Player::getTargetVelocityX() const {
    qreal targetVelocity = 0;

    if (keyLeft) {
        targetVelocity -= m_moveSpeed;
    }

    if (keyRight) {
        targetVelocity += m_moveSpeed;
    }

    return targetVelocity;
}

void Player::updateRotate(TerrainGenerator* GTerrainGenerator) {
    //当在空中且按下Space键时，旋转,若没按下，则缓慢回到地形角度
    if (isOnGround() || !keySpace) {
        // 计算当前角度归一化值（角度/360）
        qreal normalizedAngle = rotation / 360.0;

        // 目标角度固定为60度
        qreal targetAngle = 60.0;

        // 以0.5度的角速度平滑过渡到目标角度
        qreal rotateSpeed = 0.5;
        qreal angle = rotation;

        if (normalizedAngle < targetAngle) {
            angle += rotateSpeed;
            if (angle > targetAngle) {
                angle = targetAngle;
            }
        } else if (normalizedAngle > targetAngle) {
            angle -= rotateSpeed;
            if (angle < targetAngle) {
                angle = targetAngle;
            }
        }

        setRotation(angle);
        return;
    }
    //如果在空中且按下Space键，则以成员变量的角速度顺时针旋转
    qreal angle = rotation;
    angle -= rotateSpeed; // 改为减法，实现顺时针旋转

    //旋转
    setRotation(angle);
}

void Player::playerUpdate(TerrainGenerator* GTerrainGenerator) {
    // 处理旋转
    updateRotate(GTerrainGenerator);

}


void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // 先调用父类绘制红色方块
    QGraphicsRectItem::paint(painter, option, widget);

    // 绘制底部绿色边
    QRectF r = rect();
    QPen greenPen(Qt::green, 4); // 4像素宽绿色线
    painter->setPen(greenPen);
    painter->drawLine(r.bottomLeft(), r.bottomRight());
}