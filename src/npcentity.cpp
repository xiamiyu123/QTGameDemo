//
// Created by xiami on 2025/5/7.
//
// npcentity.cpp
#include "npcentity.h"
#include "groundnpc.h"
#include "flyingnpc.h"
#include "penguinnpc.h"
#include "yetimnpc.h"
#include "debuglogger.h"
#include <QBrush>
#include <QPen>
#include <QRandomGenerator>
#include <QGraphicsScene>

// === NPCEntity实现 ===

NPCEntity::NPCEntity(NPCType type, qreal width, qreal height, QGraphicsItem *parent)
    : BasePhysicsEntity(width, height, parent),
      m_npcType(type),
      m_movementSpeed(100),
      m_aiUpdateTimer(0),
      m_isActive(true),
      m_shouldDestroy(false)
{
    // 设置实体类型
    setEntityType(EntityType::NPC);    // NPC默认外观（子类可以重写）
    setBrush(QBrush(Qt::blue));
    setPen(QPen(Qt::black, 1));
    
    // 子类需要实现initializeNPC()
}

qreal NPCEntity::getTargetVelocityX() const
{
    if (!m_isActive) {
        return 0.0;
    }
    
    // 强制向右移动，返回固定的正向速度
    return m_movementSpeed;
}

void NPCEntity::resetNPC()
{
    // 重置NPC到初始状态（用于对象池复用）
    m_aiUpdateTimer = 0;
    m_isActive = true;
    m_shouldDestroy = false;
    
    // 重置物理状态
    setVelocity(QPointF(0, 0));
    setOnGround(false);
}

void NPCEntity::updatePhysics(float deltaTime)
{
    if (!m_isActive) {
        return;
    }

    // 更新AI逻辑
    updateAI(deltaTime);
    
    // 更新特殊能力
    updateSpecialAbility(deltaTime);
    
    // 更新外观
    updateAppearance(deltaTime);
    
    // 调用基类物理更新
    BasePhysicsEntity::updatePhysics(deltaTime);
}

void NPCEntity::updateSpecialAbility(float deltaTime)
{
    // 纯虚函数 - 子类必须实现
}

void NPCEntity::updateAI(float deltaTime)
{
    // 纯虚函数 - 子类必须实现
}

void NPCEntity::updateDefaultMovement()
{
    // 强制所有NPC向右移动 - 无需设置方向，getTargetVelocityX()已处理
    // 这个方法保留为空，供子类扩展其他移动逻辑
}

// === NPCFactory实现 ===

int NPCFactory::s_poolSize = 50;  // 默认对象池大小

NPCFactory::NPCFactory(QObject* parent)
    : QObject(parent)
{
}

NPCFactory::~NPCFactory()
{
}

std::unique_ptr<NPCEntity> NPCFactory::createGroundNPC(const QPointF& position)
{
    // GroundNPC现在是抽象基类，不能直接实例化
    // 目前只支持企鹅NPC作为地面NPC的具体实现
    return createPenguinNPC(position);
}

std::unique_ptr<NPCEntity> NPCFactory::createFlyingNPC(const QPointF& position)
{
    // FlyingNPC现在是抽象基类，不能直接实例化
    // 暂时返回nullptr，直到有具体的飞行NPC实现
    DEBUG_LOG("FlyingNPC is now abstract, cannot create instance");
    return nullptr;
}

std::unique_ptr<NPCEntity> NPCFactory::createPenguinNPC(const QPointF& position)
{
    auto npc = std::make_unique<PenguinNPC>();
    npc->setPosition(position);
    npc->initializeNPC();
    return npc;
}

std::unique_ptr<NPCEntity> NPCFactory::createYetiNPC(const QPointF& position)
{
    auto npc = std::make_unique<YetiNPC>();
    npc->setPosition(position);
    npc->initializeNPC();
    return npc;
}

void NPCFactory::configureNPCDefaults(NPCEntity* npc, NPCEntity::NPCType type)
{
    // 现在由子类自己配置默认属性
}