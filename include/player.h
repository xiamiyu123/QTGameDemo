#pragma once

#include "basephysicsentity.h"
#include <QKeyEvent>
#include <QTimer>
#include <QPixmap>
#include <QVector>
#include "terraingenerator.h"

class RockEntity; // 前向声明

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
    qreal getFlipRotation() const;    // 判断玩家是否处于摔倒状态
    bool isFallen() const;

    // 设置图像缩放因子
    void setImageScaleFactor(qreal factor);
    qreal imageScaleFactor() const;

    // 常量
    static const qreal MAX_LANDING_ANGLE_DEVIATION; // 最大允许着陆角度偏差

    // 设置速度倍数（用于奖励加速）
    void setSpeedMultiplier(qreal multiplier);
    qreal getSpeedMultiplier() const;

signals:
    // // 玩家摔倒信号（测试用）
    // void playerFallen(int points, const QString& reason);

protected:
    // 根据输入计算目标速度
    qreal getTargetVelocityX() const override;

    void updateRotate(TerrainGenerator *GTerrainGenerator);

private:
    // 摔倒相关方法
    void fall();  // 进入摔倒状态
    void recoverFromFall(); // 从摔倒中恢复
    bool canResistFall(qreal angleDeviation) const; // 是否能抵抗摔倒

    // 动画相关方法
    void loadAnimationFrames(); // 加载动画帧
    int getCurrentAnimationRange() const; // 根据当前状态返回应该显示的帧索引
private slots:
    void onFallRecoveryTimeout(); // 摔倒恢复计时器回调
    void updateAnimation(); // 动画更新槽

private:
    // 基本状态
    bool is_fallen;
    bool keyLeft;
    bool keyRight;
    bool keySpace;
    qreal rotateSpeed;
    qreal m_moveSpeed;
    qreal m_jumpForce;
    qreal m_speedMultiplier;// 速度倍数（用于奖励加速）

    // 空翻角度记录
    qreal m_takeoffRotation;   // 离地时的角度
    qreal m_flipRotation;      // 计算出的空翻总角度
    qreal m_cumulativeRotation; // 累计旋转角度
    qreal m_lastFrameRotation;  // 上一帧的角度

    // 摔倒恢复计时器
    QTimer m_fallRecoveryTimer;    // 动画系统 - 新增部分
    QVector<QPixmap> m_animationFrames;  // 存储png1-png38的动画帧
    int m_currentFrame;                  // 当前播放的帧索引
    QTimer m_animationTimer;            // 动画播放定时器
    bool m_animationLoaded;             // 动画是否成功加载的标志
    qreal m_imageScaleFactor;           // 图像缩放因子，用于调整显示大小

    // 移除旧的动画相关变量
    /*
    enum AnimationState {
        Standing,
        Running,
        Jumping,
        Falling,
        Landing,
        Flipping
    };
    QVector<QPixmap> m_playerImages; // 玩家图像资源
    int m_currentImageIndex; // 当前图像索引
    AnimationState m_animState; // 动画状态
    float m_animTimer; // 动画计时器
    bool m_facingRight; // 朝向
    */
};