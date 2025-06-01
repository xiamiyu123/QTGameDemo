#include "yetimnpc.h"
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QColor>
#include "debuglogger.h"

YetiNPC::YetiNPC(QGraphicsItem *parent)
    : GroundNPC(parent)
{
    // 设置雪怪的尺寸（竖直长方体）
    setRect(0, 0, YETI_WIDTH, YETI_HEIGHT);
    
    // 设置雪怪的外观（白色/浅蓝色系）
    setBrush(QBrush(QColor(230, 240, 255))); // 浅蓝白色代表雪怪身体
    setPen(QPen(QColor(200, 220, 240), 3)); // 稍深的蓝白色边框
    
    // 设置移动速度为玩家的80%
    setMovementSpeed(YETI_SPEED);
    
    DEBUG_LOG("YetiNPC created");
}

void YetiNPC::initializeNPC()
{
    // 调用父类初始化
    GroundNPC::initializeNPC();
    
    // 雪怪特有的初始化
    setMovementSpeed(YETI_SPEED);
    setActive(false); // 初始状态不激活，等待进入画面
    
    // 确保重力为0，防止漂浮在空中
    getPhysicsComponent()->setGravity(0);
    
    DEBUG_LOG("YetiNPC initialized");
}

void YetiNPC::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    
    // 绘制雪怪身体（竖直长方体）
    painter->setBrush(QBrush(QColor(230, 240, 255))); // 浅蓝白色
    painter->setPen(QPen(QColor(200, 220, 240), 3));
    painter->drawRect(0, 0, YETI_WIDTH, YETI_HEIGHT);
    
    // 绘制雪怪的眼睛
    painter->setBrush(QBrush(Qt::black));
    painter->setPen(QPen(Qt::black, 1));
    painter->drawEllipse(8, 15, 6, 6);   // 左眼
    painter->drawEllipse(26, 15, 6, 6);  // 右眼
    
    // 绘制雪怪的嘴巴
    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(15, 30, 25, 30);   // 简单的横线嘴巴
    
    // 绘制雪怪的手臂（简单的线条）
    painter->setPen(QPen(QColor(200, 220, 240), 4));
    painter->drawLine(0, 25, -8, 20);    // 左臂
    painter->drawLine(YETI_WIDTH, 25, YETI_WIDTH + 8, 20); // 右臂
}

QRectF YetiNPC::boundingRect() const
{
    // 包括手臂在内的边界
    return QRectF(-10, 0, YETI_WIDTH + 20, YETI_HEIGHT);
}
