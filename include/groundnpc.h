// groundnpc.h
#pragma once
#include "npcentity.h"

/**
 * GroundNPC: 地面NPC抽象基类
 * 为所有地面类型的NPC提供基础实现
 * 这是一个抽象类，不应该被直接实例化
 */
class GroundNPC : public NPCEntity
{
    Q_OBJECT

protected:
    // 受保护的构造函数，防止直接实例化
    explicit GroundNPC(QGraphicsItem *parent = nullptr);
    explicit GroundNPC(qreal width, qreal height, QGraphicsItem *parent = nullptr);

public:
    virtual ~GroundNPC() = default;

    // 重写初始化方法
    void initializeNPC() override;
    
    // 重写重置方法
    void resetNPC() override;

protected:
    // 重写特殊能力更新
    void updateSpecialAbility(float deltaTime) override;
    
    // 重写AI逻辑
    void updateAI(float deltaTime) override;
    
    // 重写外观更新
    void updateAppearance(float deltaTime) override;

protected:
    qreal m_animationTimer;      // 动画计时器
};
