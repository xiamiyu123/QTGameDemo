#pragma once

#include "groundnpc.h"

/**
 * PenguinNPC: 企鹅NPC
 * 继承自GroundNPC，实现企鹅特有的行为
 * 速度为玩家的一半，其他保持默认
 */
class PenguinNPC : public GroundNPC
{
    Q_OBJECT

public:
    explicit PenguinNPC(QGraphicsItem *parent = nullptr);
    ~PenguinNPC() override = default;

    // 重写初始化方法
    void initializeNPC() override;

    // 重写绘制方法以显示企鹅外观
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    // 企鹅特有的常量
    static constexpr qreal PENGUIN_SPEED = 250.0; // 玩家速度的一半
    static constexpr qreal PENGUIN_WIDTH = 30.0;
    static constexpr qreal PENGUIN_HEIGHT = 30.0;
};
