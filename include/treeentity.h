#pragma once

#include <QGraphicsRectItem>
#include <QPainter>
#include <QRandomGenerator>

/**
 * 树木实体类
 * 随机生成不同形状的树木，用于装饰地形
 */
class TreeEntity : public QGraphicsRectItem
{
public:
    explicit TreeEntity(qreal width = 40, qreal height = 80, QGraphicsItem* parent = nullptr);
    
    // 设置树木位置
    void setPosition(const QPointF& position);
    
    // 获取位置信息
    qreal x() const { return pos().x(); }
    qreal y() const { return pos().y(); }
    
    // 重写绘制方法，绘制树木形状
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    qreal m_width;      // 树木宽度
    qreal m_height;     // 树木高度
    int m_treeType;     // 树木类型（用于生成不同形状）
    QColor m_trunkColor; // 树干颜色
    QColor m_leavesColor; // 树叶颜色
    QColor m_snowColor;  // 雪的颜色
      // 绘制不同类型的雪原树木 - 保留两种浪漫的树型
    void drawSnowPine(QPainter* painter);       // 雪松 - 厚重积雪的松树
    void drawDeadTree(QPainter* painter);       // 枯树 - 优雅的冬季落叶树
};
