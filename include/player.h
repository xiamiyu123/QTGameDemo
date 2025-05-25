#pragma once
#include "basephysicsentity.h"
#include <QKeyEvent>
#include <QTimer>

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

    void setOnGround(bool onGround) override;

    // 新增方法：检查落地角度并判断是否摔倒
    void checkLanding(qreal terrainAngle);

    // 检查玩家是否与石头碰撞并判断是否摔倒
    void checkHitRock(RockEntity* rock);

    // 记录起跳和离地信息
    void notifyTakeoff();
    
    void playerUpdate(TerrainGenerator* GTerrainGenerator);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    
    // 获取最后一次空翻角度
    qreal getFlipRotation() const;
    // 绘制偏移
    qreal m_drawOffsetX;
    qreal m_drawOffsetY;
    
    // 判断玩家是否处于摔倒状态
    bool isFallen() const;

protected:
    // 根据输入计算目标速度
    qreal getTargetVelocityX() const override;

    void updateRotate(TerrainGenerator *GTerrainGenerator);

private:
    // 摔倒相关方法
    void fall();  // 进入摔倒状态
    void recoverFromFall(); // 从摔倒中恢复
    bool canResistFall(qreal angleDeviation) const; // 是否能抵抗摔倒

private slots:
    void onFallRecoveryTimeout(); // 摔倒恢复计时器回调

private:
    //是否摔倒
    bool is_fallen;
    bool keyLeft;
    bool keyRight;
    bool keySpace;
    qreal rotateSpeed;
    qreal m_moveSpeed;
    qreal m_jumpForce;
    
    // 空翻角度记录
    qreal m_takeoffRotation;   // 离地时的角度
    qreal m_flipRotation;      // 计算出的空翻总角度
    qreal m_cumulativeRotation; // 累计旋转角度
    qreal m_lastFrameRotation;  // 上一帧的角度
    
    // 摔倒恢复计时器
    QTimer m_fallRecoveryTimer;
    
    // 常量
    static const qreal MAX_LANDING_ANGLE_DEVIATION; // 最大允许着陆角度偏差
    enum AnimationState {
        Standing,
        Running,
        Jumping,
        Falling,
        Landing,
        Flipping
    };

    // 图像资源
    QVector<QPixmap> m_playerImages; // 玩家图像资源
    int m_currentImageIndex; // 当前图像索引
    AnimationState m_animState; // 动画状态
    float m_animTimer; // 动画计时器


    // 朝向
    bool m_facingRight;

    // 动画处理
    void updateAnimation();
};
