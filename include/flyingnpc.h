// flyingnpc.h
#pragma once
#include "npcentity.h"

/**
 * FlyingNPC: 飞行NPC抽象基类
 * 为所有飞行类型的NPC提供基础实现
 * 这是一个抽象类，不应该被直接实例化
 */
class FlyingNPC : public NPCEntity
{
    Q_OBJECT

protected:
    // 受保护的构造函数，防止直接实例化
    explicit FlyingNPC(QGraphicsItem *parent = nullptr);
    explicit FlyingNPC(qreal width, qreal height, QGraphicsItem *parent = nullptr);

public:
    virtual ~FlyingNPC() = default;

    // 重写初始化方法
    void initializeNPC() override;
    
    // 重写重置方法
    void resetNPC() override;

protected:
    // 重写特殊能力更新 - 简单飞行逻辑
    void updateSpecialAbility(float deltaTime) override;
    
    // 重写AI逻辑
    void updateAI(float deltaTime) override;
    
    // 重写外观更新
    void updateAppearance(float deltaTime) override;

protected:
    qreal m_animationTimer;      // 动画计时器
};
