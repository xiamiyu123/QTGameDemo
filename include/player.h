#pragma once
#include "basephysicsentity.h"
#include <QKeyEvent>

#include "terraingenerator.h"

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
    void playerUpdate(TerrainGenerator* GTerrainGenerator);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

protected:
    // 根据输入计算目标速度
    qreal getTargetVelocityX() const override;

    void updateRotate(TerrainGenerator *GTerrainGenerator);

private:
    //是否摔倒
    bool is_fallen;
    bool keyLeft;
    bool keyRight;
    bool keySpace;
    qreal rotateSpeed;
    qreal m_moveSpeed;
    qreal m_jumpForce;
};
