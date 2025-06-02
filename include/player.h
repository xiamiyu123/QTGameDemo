#pragma once

#include "basephysicsentity.h"
#include <QKeyEvent>
#include <QTimer>
#include <QPixmap>
#include <QVector>
#include <queue>  // 添加优先队列支持
#include "terraingenerator.h"

class RockEntity; // 前向声明
class NPCEntity;  // 添加NPC前向声明

class Player : public BasePhysicsEntity
{
    Q_OBJECT

public:    // NPC形态枚举
    enum class NPCForm {
        Normal,        // 普通形态（无NPC）
        Penguin,       // 企鹅形态
        YetiForm1,     // 雪怪形态1（初始骑乘）
        YetiForm2,     // 雪怪形态2（被撞击后）
        // 可以继续添加其他形态
    };    // 属性加成配置结构
    struct NPCFormModifiers {
        qreal moveSpeedMultiplier = 1.0;    // 移动速度倍率
        qreal jumpForceMultiplier = 1.0;    // 跳跃力倍率
        qreal flipSpeedMultiplier = 1.0;    // 空翻速度倍率
        int inventoryCapacityBonus = 0;     // 库存容量加成
        bool canFlip = true;                // 是否可以空翻
        int penguinCarryCapacity = 0;       // 额外的企鹅携带容量（仅携带企鹅，不提供属性加成）
    };

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
    void checkHitRock(RockEntity* rock);    // 新增：NPC拾取和丢弃方法
    bool pickupNPC(NPCEntity* npc); // 返回是否成功拾取
    void dropNPC();
    void dropNPC(TerrainGenerator* terrainGenerator); // 需要地形生成器来创建NPC
    bool hasNPCInInventory() const;
    int getInventorySize() const;
      // NPC拾取冷却相关方法
    bool canPickupNPC() const;        // 检查是否可以拾取NPC（冷却状态）
    void startNPCPickupCooldown();    // 启动拾取冷却
    qreal getNPCPickupCooldownProgress() const; // 获取冷却进度(0.0-1.0)

    // 记录起跳和离地信息
    void notifyTakeoff();

    void playerUpdate(TerrainGenerator* GTerrainGenerator);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    // 获取最后一次空翻角度
    qreal getFlipRotation() const;    // 判断玩家是否处于摔倒状态
    bool isFallen() const;    // 设置图像缩放因子
    void setImageScaleFactor(qreal factor);
    qreal imageScaleFactor() const;

    // 新增：NPC形态系统
    NPCForm getCurrentForm() const;
    void applyNPCForm(NPCForm form);
    void updatePlayerAttributes();
    qreal getCurrentMoveSpeed() const;
    qreal getCurrentJumpForce() const;    qreal getCurrentFlipSpeed() const;
    int getCurrentInventoryCapacity() const;

    // 雪怪形态管理
    bool isRidingYeti() const;
    NPCForm getYetiForm() const;
    void transformYetiForm(); // 雪怪形态1转为形态2
    bool canFlip() const; // 检查当前是否可以空翻
      // 企鹅携带系统
    void addPenguinToCarry(int penguinId); // 添加企鹅到携带库存
    bool consumePenguinForDamageResistance(); // 消耗携带的企鹅抵抗伤害
    void dropCarriedPenguin(); // 丢弃携带的企鹅
    int getCarriedPenguinCount() const; // 获取携带的企鹅数量

    // 常量
    static const qreal MAX_LANDING_ANGLE_DEVIATION; // 最大允许着陆角度偏差

    void setMoveSpeed(qreal speed);
    qreal moveSpeed() const;

    // 开始空翻加速效果
    void startFlipBoost();

    // 检查是否处于空翻加速状态
    bool isFlipBoosting() const;

signals:
    // // 玩家摔倒信号（测试用）
    // void playerFallen(int points, const QString& reason);
    // // 新增空翻成功信号
    void backflipSuccess(int points, const QString& message);

protected:
    // 根据输入计算目标速度
    qreal getTargetVelocityX() const override;

    void updateRotate(TerrainGenerator *GTerrainGenerator);

private:
    // 摔倒相关方法
    void fall();  // 进入摔倒状态
    void recoverFromFall(); // 从摔倒中恢复
    bool canResistFall(qreal angleDeviation); // 是否能抵抗摔倒（可消耗NPC进行抗性）    // 动画相关方法
    void loadAnimationFrames(); // 加载动画帧
    int getCurrentAnimationRange() const; // 根据当前状态返回应该显示的帧索引

signals:
    void updatePlayerNPC(); // NPC增减操作时发出的信号
    void npcPickupCooldownChanged(bool active, qreal progress); // NPC拾取冷却状态变化信号
    void fallRecoveryChanged(bool active, qreal progress); // 摔倒恢复进度信号
    void flipBoostChanged(bool active, qreal progress); // 空翻加速进度信号

private slots:
    void onFallRecoveryTimeout(); // 摔倒恢复计时器回调
    void updateAnimation(); // 动画更新槽
    void onFlipBoostTimerTimeout(); // 空翻加速计时器回调
    void onNPCPickupCooldownTimeout(); // NPC拾取冷却计时器回调
    void updateNPCCooldownProgress(); // 更新NPC拾取冷却进度
    void updateFallRecoveryProgress(); // 更新摔倒恢复进度
    void updateFlipBoostProgress(); // 更新空翻加速进度
    void onUpdate(); // 更新玩家状态的槽函数

private:
    // 基本状态
    bool is_fallen;
    bool keyLeft;
    bool keyRight;
    bool keySpace;
    qreal rotateSpeed;
    qreal m_moveSpeed;
    qreal m_jumpForce;

    bool consumeNPCForDamageResistance(); // 消耗NPC进行伤害抵抗

    // 空翻角度记录
    qreal m_takeoffRotation;   // 离地时的角度
    qreal m_flipRotation;      // 计算出的空翻总角度
    qreal m_cumulativeRotation; // 累计旋转角度
    qreal m_lastFrameRotation;  // 上一帧的角度    // 空翻加速相关
    QTimer* m_flipBoostTimer;  // 空翻加速计时器
    QTimer* m_flipBoostProgressTimer; // 空翻加速进度更新计时器
    bool m_isFlipBoosting;     // 当前是否处于空翻加速状态
    static const int FLIP_BOOST_TIME_MS = 2000; // 2秒加速时间
    static const int FLIP_BOOST_PROGRESS_UPDATE_MS = 50; // 进度更新间隔

    // 摔倒恢复计时器
    QTimer m_fallRecoveryTimer;    // 动画系统 - 新增部分
    QTimer m_fallRecoveryProgressTimer; // 摔倒恢复进度更新定时器// 动画系统 - 新增部分
    QVector<QPixmap> m_animationFrames;  // 存储png1-png38的动画帧
    int m_currentFrame;                  // 当前播放的帧索引
    QTimer m_animationTimer;            // 动画播放定时器
    bool m_animationLoaded;             // 动画是否成功加载的标志
    qreal m_imageScaleFactor;           // 图像缩放因子，用于调整显示大小    // NPC库存系统 - 使用优先队列实现堆（降序排列，高ID优先）
    std::priority_queue<int> m_npcInventory; // 存储NPC ID，自动按ID降序排列
    static const int MAX_INVENTORY_SIZE = 1; // 最大库存大小
      // NPC拾取冷却系统
    QTimer m_npcPickupCooldownTimer; // NPC拾取冷却计时器
    QTimer m_npcCooldownProgressTimer; // 冷却进度更新计时器
    bool m_npcPickupCooldownActive;  // 拾取冷却是否激活
    static const int NPC_PICKUP_COOLDOWN_MS = 1000; // 1秒冷却时间
    static const int NPC_COOLDOWN_PROGRESS_UPDATE_MS = 50; // 进度更新间隔（20FPS）

    // 地形生成器引用，用于NPC丢弃功能
    TerrainGenerator* m_terrainGenerator;

    // NPC形态系统
    NPCForm m_currentForm;

    // 基础属性值（原始值）
    qreal m_baseMoveSpeed;
    qreal m_baseJumpForce;
    qreal m_baseFlipSpeed;
    int m_baseInventoryCapacity;

    // 当前生效的属性值（包含加成）
    qreal m_currentMoveSpeed;    qreal m_currentJumpForce;
    qreal m_currentFlipSpeed;
    int m_currentInventoryCapacity;

    // 雪怪形态管理
    bool m_isRidingYeti;                // 是否正在骑乘雪怪
    NPCForm m_yetiForm;                 // 当前雪怪形态
    std::priority_queue<int> m_penguinCarryInventory; // 额外的企鹅携带库存（仅抵抗伤害用）
      // 各种形态的属性配置
    static const NPCFormModifiers PENGUIN_MODIFIERS;
    static const NPCFormModifiers YETI_FORM1_MODIFIERS;
    static const NPCFormModifiers YETI_FORM2_MODIFIERS;

    // NPC形态相关私有方法
    void resetToNormalForm();
    NPCFormModifiers getFormModifiers(NPCForm form) const;
    void applyFormModifiers(const NPCFormModifiers& modifiers);
    void updatePlayerFormBasedOnInventory();
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