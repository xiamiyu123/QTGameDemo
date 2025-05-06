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

    void move(qreal dx, qreal dy);
    void update();
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

private:
    qreal m_velocityX;
    qreal m_velocityY;
    bool m_movingLeft;
    bool m_movingRight;
    bool m_onGround;

    PhysicsComponent* m_physicsComponent;
};