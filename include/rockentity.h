#pragma once
#include "basephysicsentity.h"
#include <QBrush>
#include <QPen>
#include <QPixmap>
#include <QPainter>

class RockEntity : public BasePhysicsEntity
{
    Q_OBJECT
public:
    RockEntity(qreal width = 30, qreal height = 30, QGraphicsItem* parent = nullptr);
    ~RockEntity() override = default;

    // 禁止物理系统更新石头位置
    void updatePhysics(float) override {}

    // 重写绘制方法以支持贴图
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    void loadRockTexture();

    QPixmap m_rockTexture;  // 石头贴图
    bool m_textureLoaded;   // 贴图是否加载成功
};