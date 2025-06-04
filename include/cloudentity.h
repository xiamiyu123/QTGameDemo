#pragma once

#include <QGraphicsRectItem>
#include <QPainter>
#include <QRandomGenerator>
#include <QTimer>

/**
 * 云朵实体类
 * 随机生成不同形状的云朵，带有简单的漂浮动画
 */
class CloudEntity : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
    
public:
    explicit CloudEntity(qreal width = 120, qreal height = 60, QGraphicsItem* parent = nullptr);
    
    // 设置云朵位置
    void setPosition(const QPointF& position);
    
    // 获取位置信息
    qreal x() const { return pos().x(); }
    qreal y() const { return pos().y(); }
    
    // 重写绘制方法，绘制云朵形状
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    
    // 启动漂浮动画
    void startFloating();
    
    // 停止漂浮动画
    void stopFloating();

private slots:
    void updateFloating(); // 更新漂浮位置

private:
    qreal m_width;          // 云朵宽度
    qreal m_height;         // 云朵高度
    int m_cloudType;        // 云朵类型（用于生成不同形状）
    QColor m_cloudColor;    // 云朵颜色
    
    // 漂浮动画相关
    QTimer* m_floatTimer;   // 漂浮动画定时器
    qreal m_baseY;          // 基础Y坐标
    qreal m_floatOffset;    // 当前漂浮偏移
    qreal m_floatSpeed;     // 漂浮速度
    
    // 绘制不同类型的云朵
    void drawCumulusCloud(QPainter* painter);     // 积云
    void drawStratusCloud(QPainter* painter);     // 层云
    void drawCirrusCloud(QPainter* painter);      // 卷云
};
