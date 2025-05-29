#pragma once

#include "basephysicsentity.h"
#include <QKeyEvent>
#include <QTimer>
#include <QPixmap>
#include <QVector>
#include <QString>
#include <queue>
#include <functional>
#include <set>
#include <vector>
#include "terraingenerator.h"
#include "npcentity.h"

class RockEntity; // 前向声明

// === NPC携带系统相关 ===
/**
 * CarriedNPC: 被携带的NPC信息
 */
struct CarriedNPC {
    NPCEntity* npc;          // NPC指针
    NPCCarryEffect effect;   // 携带效果
    int priority;            // 优先级
    
    CarriedNPC(NPCEntity* n, const NPCCarryEffect& e, int p)
        : npc(n), effect(e), priority(p) {}
    
    // 优先级比较器（用于最大堆，优先级高的在前）
    bool operator<(const CarriedNPC& other) const {
        return priority < other.priority;
    }
};

/**
 * NPCCarryManager: NPC携带管理器
 * 使用优先级队列管理携带的NPC
 */
class NPCCarryManager {
public:
    NPCCarryManager() = default;
    ~NPCCarryManager() = default;
    
    // 添加携带的NPC
    void addNPC(NPCEntity* npc);
    
    // 移除特定NPC
    bool removeNPC(NPCEntity* npc);
    
    // 获取携带数量
    int getCarriedCount() const { return static_cast<int>(m_carriedNPCs.size()); }
      // 获取最高优先级的NPC
    NPCEntity* getHighestPriorityNPC() const;
    
    // 获取最低优先级的NPC
    NPCEntity* getLowestPriorityNPC() const;
    
    // 获取最高优先级的NPC效果
    NPCCarryEffect getHighestPriorityEffect() const;
    
    // 获取总效果（可能需要叠加多个NPC的效果）
    NPCCarryEffect getTotalEffect() const;
    
    // 清空所有携带的NPC
    void clear();
    
    // 检查是否携带了特定NPC
    bool isCarrying(NPCEntity* npc) const;
    
    // 获取所有携带的NPC列表（按优先级排序）
    std::vector<NPCEntity*> getAllCarriedNPCs() const;

private:
    std::priority_queue<CarriedNPC> m_carriedNPCs;  // 优先级队列
    std::set<NPCEntity*> m_npcSet;                  // 用于快速查找的集合
};

class Player : public BasePhysicsEntity
{
    Q_OBJECT

signals:
    // NPC掉落信号，用于通知GameScene重生NPC
    void npcDropped(NPCEntity::NPCType npcType, QPointF position);

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

    // === NPC携带系统相关方法 ===
    // 拾取NPC
    bool pickupNPC(NPCEntity* npc);
    
    // 丢弃NPC（丢弃优先级最低的，或指定的NPC）
    bool dropNPC(NPCEntity* npc = nullptr);
    
    // 获取携带的NPC数量
    int getCarriedNPCCount() const;
    
    // 获取优先级最高的NPC
    NPCEntity* getHighestPriorityNPC() const;
    
    // 获取当前总的携带效果
    NPCCarryEffect getCurrentCarryEffect() const;
    
    // 检查是否已携带特定NPC
    bool isCarryingNPC(NPCEntity* npc) const;
    
    // 获取所有携带的NPC列表
    std::vector<NPCEntity*> getAllCarriedNPCs() const;
    
    // 应用携带效果到玩家属性
    void applyCarryEffects();
    
    // === 碰撞和冷却相关方法 ===
    // 判断是否可以拾取NPC（检查冷却状态）
    bool canPickupNPC() const;
    
    // 失去NPC并进入冷却状态（碰撞时调用）
    void loseNPCAndCooldown();
    
    // 判断碰撞是否会被NPC效果抵消
    bool isCollisionResisted() const;
    
    // === 调试辅助方法 ===
    // 获取携带状态的描述字符串
    QString getCarryStatusString() const;
    
    // 打印所有携带的NPC信息
    void debugPrintCarriedNPCs() const;

    // 常量
    static const qreal MAX_LANDING_ANGLE_DEVIATION; // 最大允许着陆角度偏差

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
    void onPickupCooldownEnd(); // NPC拾取冷却结束槽

private:
    // 基本状态
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
    QTimer m_fallRecoveryTimer;    // 动画系统 - 新增部分
    QVector<QPixmap> m_animationFrames;  // 存储png1-png38的动画帧
    int m_currentFrame;                  // 当前播放的帧索引
    QTimer m_animationTimer;            // 动画播放定时器
    bool m_animationLoaded;             // 动画是否成功加载的标志
    qreal m_imageScaleFactor;           // 图像缩放因子，用于调整显示大小

    // === NPC携带系统相关成员变量 ===
    NPCCarryManager m_npcCarryManager;  // NPC携带管理器
    qreal m_baseSpeed;                  // 基础移动速度（未应用效果）
    qreal m_baseJumpForce;             // 基础跳跃力（未应用效果）
    int m_maxCarryCount;               // 最大携带数量限制
    
    // NPC拾取冷却相关
    bool m_isPickupCooldown;           // 是否在冷却状态
    QTimer m_pickupCooldownTimer;      // 冷却计时器
};