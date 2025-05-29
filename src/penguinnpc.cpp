#include "penguinnpc.h"
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QColor>
#include "debuglogger.h"

PenguinNPC::PenguinNPC(QGraphicsItem *parent)
    : GroundNPC(parent)
{
    // 设置企鹅的尺寸
    setRect(0, 0, PENGUIN_WIDTH, PENGUIN_HEIGHT);
    
    // 设置企鹅的外观（黑白色系）
    setBrush(QBrush(QColor(50, 50, 50))); // 深灰色代表企鹅身体
    setPen(QPen(Qt::white, 2)); // 白色边框
    
    // 设置移动速度为玩家的一半
    setMovementSpeed(PENGUIN_SPEED);
    
    DEBUG_LOG("PenguinNPC created");
}

void PenguinNPC::initializeNPC()
{
    // 调用父类初始化
    GroundNPC::initializeNPC();
    
    // 企鹅特有的初始化
    setMovementSpeed(PENGUIN_SPEED);
    setActive(false); // 初始状态不激活，等待进入画面
    
    // === 设置携带系统相关属性 ===
    setPriority(10);  // 企鹅优先级设为10（中等优先级）
    setCarriable(true); // 企鹅可以被携带    // 设置企鹅的携带效果：增加移动速度和跳跃力，并提供碰撞抵抗
    NPCCarryEffect penguinEffect;
    penguinEffect.speedMultiplier = 1.2;      // 速度提升20%
    penguinEffect.jumpForceMultiplier = 1.1;  // 跳跃力提升10%
    penguinEffect.rotationResistance = 0.1;   // 轻微的旋转阻力
    penguinEffect.effectDescription = "企鹅伙伴：移动速度+20%，跳跃力+10%，碰撞抵抗";
    setCarryEffect(penguinEffect);
    
    DEBUG_LOG("PenguinNPC initialized");
}

void PenguinNPC::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    
    // 绘制企鹅外观
    QRectF rect = boundingRect();
    
    // 身体（黑色椭圆）
    painter->setBrush(QBrush(QColor(40, 40, 40)));
    painter->setPen(QPen(Qt::black, 1));
    painter->drawEllipse(rect.adjusted(2, 2, -2, -2));
    
    // 肚子（白色椭圆）
    QRectF belly = rect.adjusted(6, 8, -6, -4);
    painter->setBrush(QBrush(Qt::white));
    painter->drawEllipse(belly);
    
    // 眼睛
    painter->setBrush(QBrush(Qt::white));
    painter->drawEllipse(rect.x() + 8, rect.y() + 6, 4, 4);
    painter->drawEllipse(rect.x() + 18, rect.y() + 6, 4, 4);
    
    // 瞳孔
    painter->setBrush(QBrush(Qt::black));
    painter->drawEllipse(rect.x() + 9, rect.y() + 7, 2, 2);
    painter->drawEllipse(rect.x() + 19, rect.y() + 7, 2, 2);
    
    // 嘴巴（橙色小三角）
    painter->setBrush(QBrush(QColor(255, 165, 0)));
    QPolygonF beak;
    beak << QPointF(rect.x() + 15, rect.y() + 12)
         << QPointF(rect.x() + 12, rect.y() + 15)
         << QPointF(rect.x() + 18, rect.y() + 15);
    painter->drawPolygon(beak);
}
