#pragma once

#include <QTimer>

#include "groundnpc.h"

/**
 * YetiNPC: 雪怪NPC
 * 继承自GroundNPC，实现雪怪特有的行为
 * 速度为玩家的80%，优先级ID为2，拥有两种骑乘形态
 * 未加载图片时雪怪为竖直长方体形状
 */
class YetiNPC : public GroundNPC
{
    Q_OBJECT

public:
    explicit YetiNPC(QGraphicsItem *parent = nullptr);
    ~YetiNPC() override = default;

    // 雪怪的优先级ID
    static constexpr int ID = 2;

    int class_id() const override {
        return ID;
    }

    // 重写初始化方法
    void initializeNPC() override;

    // 重写绘制方法以显示雪怪外观
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    QRectF boundingRect() const override;

private:
    // 雪怪特有的常量
    static constexpr qreal YETI_SPEED = 400.0; // 玩家速度的80% (玩家500*0.8)
    static constexpr qreal YETI_WIDTH = 40.0;  // 竖直长方体：较窄的宽度
    static constexpr qreal YETI_HEIGHT = 60.0; // 竖直长方体：较高的高度
    // 贴图相关
    QVector<QPixmap> m_animationFrames;  // 动画帧
    int m_currentFrame;                  // 当前帧
    QTimer m_animationTimer;            // 动画定时器
    bool m_textureLoaded;               // 贴图是否加载成功

    void loadAnimationFrames();         // 加载动画帧
    void updateAnimation();
};
