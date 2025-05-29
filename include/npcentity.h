// npcentity.h
#pragma once
#include "basephysicsentity.h"
#include <QObject>
#include <QGraphicsItem>
#include <memory>

// 前向声明
class QGraphicsScene;

// === NPC携带效果结构体 ===
/**
 * NPCCarryEffect: NPC被携带时对玩家的影响效果
 */
struct NPCCarryEffect {
    qreal speedMultiplier = 1.0;      // 移动速度倍数 (1.0 = 无变化)
    qreal jumpForceMultiplier = 1.0;  // 跳跃力倍数
    qreal gravityMultiplier = 1.0;    // 重力倍数
    qreal rotationResistance = 0.0;   // 旋转阻力 (0.0-1.0, 0表示无阻力)
      // 特殊效果标识
    bool enableDoubleJump = false;    // 是否允许二段跳
    bool enableGliding = false;       // 是否允许滑翔
    
    // 效果描述（用于UI显示）
    QString effectDescription;
    
    // 默认构造函数
    NPCCarryEffect() = default;
    
    // 便利构造函数
    NPCCarryEffect(qreal speed, qreal jump = 1.0, const QString& desc = "")
        : speedMultiplier(speed), jumpForceMultiplier(jump), effectDescription(desc) {}
};

/**
 * NPCEntity: NPC基类
 * 为游戏中的NPC提供基础实现，支持AI行为
 * 设计为易于继承，支持不同类型的NPC（飞鸟、陆地动物等）
 */
class NPCEntity : public BasePhysicsEntity
{
    Q_OBJECT

public:
    // NPC类型枚举
    enum class NPCType {
        Ground,      // 陆地动物（如企鹅、雪怪等）
        Flying,      // 飞鸟
        // 可扩展其他类型
    };

    // 构造函数 - 使用explicit避免隐式转换
    explicit NPCEntity(NPCType type, qreal width = 25, qreal height = 25, QGraphicsItem *parent = nullptr);
    virtual ~NPCEntity() = default;    // === 基础属性访问 ===
    NPCType getNPCType() const { return m_npcType; }
    void setMovementSpeed(qreal speed) { m_movementSpeed = speed; }
    qreal getMovementSpeed() const { return m_movementSpeed; }

    // === NPC携带系统相关 ===
    // 优先级属性（数值越大优先级越高）
    int getPriority() const { return m_priority; }
    void setPriority(int priority) { m_priority = priority; }
    
    // 携带效果属性
    const NPCCarryEffect& getCarryEffect() const { return m_carryEffect; }
    void setCarryEffect(const NPCCarryEffect& effect) { m_carryEffect = effect; }
    
    // 是否可以被携带
    bool isCarriable() const { return m_isCarriable; }
    void setCarriable(bool carriable) { m_isCarriable = carriable; }
    
    // 是否已被携带
    bool isCarried() const { return m_isCarried; }
    void setCarried(bool carried) { m_isCarried = carried; }

    // === NPC生命周期管理 ===
    bool isActive() const { return m_isActive; }
    void setActive(bool active) { m_isActive = active; }
    
    // 标记为待删除（供对象池回收使用）
    void markForDestroy() { m_shouldDestroy = true; }
    bool shouldDestroy() const { return m_shouldDestroy; }    // === 纯虚函数接口供子类必须实现 ===
    // 初始化NPC（在创建后调用）
    virtual void initializeNPC() = 0;
    
    // 重置NPC状态（用于对象池复用）
    virtual void resetNPC() = 0;

protected:
    // 获取AI控制的目标速度
    qreal getTargetVelocityX() const override;    // === 纯虚函数供子类必须实现 ===
    // NPC特殊能力更新（如飞鸟的飞行逻辑）
    virtual void updateSpecialAbility(float deltaTime) = 0;
    
    // AI逻辑更新
    virtual void updateAI(float deltaTime) = 0;
    
    // 更新外观和动画
    virtual void updateAppearance(float deltaTime) = 0;

    // 物理更新重写 - 整合AI和特殊能力
    void updatePhysics(float deltaTime) override;    // === 受保护的属性供子类访问 ===
    NPCType m_npcType;
    qreal m_movementSpeed;
    qreal m_aiUpdateTimer;
    bool m_isActive;
    bool m_shouldDestroy;
    
    // === 携带系统相关属性 ===
    int m_priority;                   // 优先级（默认为0）
    NPCCarryEffect m_carryEffect;     // 携带效果
    bool m_isCarriable;              // 是否可被携带（默认true）
    bool m_isCarried;                // 是否已被携带（默认false）

private:
    // 默认AI行为 - 强制向右移动
    void updateDefaultMovement();
};

// === NPC工厂类 ===
/**
 * NPCFactory: NPC工厂类
 * 提供统一的NPC创建和管理接口
 */
class NPCFactory : public QObject
{
    Q_OBJECT

public:
    explicit NPCFactory(QObject* parent = nullptr);
    ~NPCFactory();    // === 工厂方法 ===
    // 创建地面动物NPC
    static std::unique_ptr<NPCEntity> createGroundNPC(const QPointF& position = QPointF(0, 0));
    
    // 创建飞鸟NPC
    static std::unique_ptr<NPCEntity> createFlyingNPC(const QPointF& position = QPointF(0, 0));
    
    // 创建企鹅NPC
    static std::unique_ptr<NPCEntity> createPenguinNPC(const QPointF& position = QPointF(0, 0));

    // === 对象池管理（为后续扩展预留） ===
    // 设置对象池大小
    static void setPoolSize(int size) { s_poolSize = size; }
    static int getPoolSize() { return s_poolSize; }

private:
    static int s_poolSize;  // 对象池大小
    
    // 辅助方法：配置NPC基础属性
    static void configureNPCDefaults(NPCEntity* npc, NPCEntity::NPCType type);
};