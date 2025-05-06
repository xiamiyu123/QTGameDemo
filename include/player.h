#pragma once
#include <QGraphicsRectItem>
#include <QObject>
#include <QKeyEvent>
#include "physical.h"  // 包含物理系统头文件

class Player : public QObject, public QGraphicsRectItem, public IPhysicsObject
{
    Q_OBJECT

public:
    Player(QGraphicsItem *parent = nullptr);
    ~Player() override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    // 实现IPhysicsObject接口
    QPointF position() const override;
    void setPosition(const QPointF& pos) override;
    QPointF velocity() const override;
    void setVelocity(const QPointF& velocity) override;
    QRectF boundingRect() const override;
    bool isOnGround() const override;
    void setOnGround(bool onGround) override;
    void updatePhysics(float deltaTime) override;

    qreal getSlopeSlideSpeed() const override;

    void setSlopeSlideSpeed(qreal speed) override;

    qreal getMoveSpeed() const override;

    void setMoveSpeed(qreal speed) override;

    bool isMovingLeft() const;

    bool isMovingRight() const;

private:
    qreal m_velocityX;
    qreal m_velocityY;
    bool m_movingLeft;
    bool m_movingRight;
    bool m_onGround;
    qreal m_moveSpeed;
    qreal m_slopeSlideSpeed;

    PhysicsComponent* m_physicsComponent;
};