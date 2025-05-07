#pragma once
#include "basephysicsentity.h"
#include <QKeyEvent>

class Player : public BasePhysicsEntity
{
    Q_OBJECT

public:
    Player(QGraphicsItem *parent = nullptr);
    ~Player() override;

    // 玩家特有的输入处理
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    // 玩家特有的跳跃方法
    void jump();

protected:
    // 根据输入计算目标速度
    qreal getTargetVelocityX() const override;

private:
    bool m_movingLeft;
    bool m_movingRight;
    qreal m_moveSpeed;
    qreal m_jumpForce;
};