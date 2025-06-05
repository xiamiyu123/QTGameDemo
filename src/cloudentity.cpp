#include "cloudentity.h"
#include <QBrush>
#include <QPen>
#include <QPolygonF>
#include <QtMath>

CloudEntity::CloudEntity(qreal width, qreal height, QGraphicsItem* parent)
    : QGraphicsRectItem(0, 0, width, height, parent)
    , m_width(width)
    , m_height(height)
    , m_baseY(0)
    , m_floatOffset(0)
    , m_floatTimer(nullptr)
{    // 随机选择云朵类型 (0=浪漫积云, 1=梦幻层云, 2=轻盈卷云)
    m_cloudType = QRandomGenerator::global()->bounded(3);
    
    // 根据云朵类型设置颜色 - 增加粉色调和梦幻感
    switch (m_cloudType) {
    case 0: // 浪漫积云 - 带粉红色调的白色厚实
        m_cloudColor = QColor(255, 248, 252, 210);
        break;
    case 1: // 梦幻层云 - 带紫色调的灰白色
        m_cloudColor = QColor(245, 240, 250, 170);
        break;
    case 2: // 轻盈卷云 - 淡蓝色调的半透明白色
        m_cloudColor = QColor(250, 252, 255, 130);
        break;
    }
      // 设置边界框透明
    setBrush(QBrush(Qt::transparent));
    setPen(QPen(Qt::transparent));
}

void CloudEntity::setPosition(const QPointF& position)
{
    m_baseY = position.y() - m_height / 2;
    setPos(position.x() - m_width / 2, m_baseY);
}



void CloudEntity::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    
    painter->setRenderHint(QPainter::Antialiasing);
    
    // 根据云朵类型绘制不同形状
    switch (m_cloudType) {
    case 0:
        drawCumulusCloud(painter);
        break;
    case 1:
        drawStratusCloud(painter);
        break;
    case 2:
        drawCirrusCloud(painter);
        break;
    }
}

void CloudEntity::startFloating()
{
    // 静态云朵，不需要动画
}

void CloudEntity::stopFloating()
{
    // 静态云朵，不需要动画
}

void CloudEntity::updateFloating()
{
    // 静态云朵，不需要动画更新
}



void CloudEntity::drawCumulusCloud(QPainter* painter)
{
    // 绘制浪漫积云（多个重叠的圆形组成蓬松效果，带有更丰富的层次感）
    // 添加淡淡的内部阴影
    QColor shadowColor = m_cloudColor.darker(105);
    shadowColor.setAlpha(80);
    painter->setBrush(QBrush(m_cloudColor));
    painter->setPen(QPen(m_cloudColor, 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    
    // 主体部分（更大更饱满）
    qreal mainRadius = m_height * 0.55;
    QRectF mainCloud(m_width * 0.25, m_height * 0.22, mainRadius * 2, mainRadius * 2);
    painter->drawEllipse(mainCloud);
    
    // 左侧大圆形 - 增加体积感
    qreal leftRadius = m_height * 0.45;
    QRectF leftCloud(m_width * 0.02, m_height * 0.32, leftRadius * 2, leftRadius * 2);
    painter->drawEllipse(leftCloud);
    
    // 右侧大圆形 - 增加跳跃感
    qreal rightRadius = m_height * 0.5;
    QRectF rightCloud(m_width * 0.57, m_height * 0.25, rightRadius * 2, rightRadius * 2);
    painter->drawEllipse(rightCloud);
    
    // 添加高光和微妙的渐变效果
    QRadialGradient gradient(m_width * 0.4, m_height * 0.3, m_width * 0.6);
    QColor highlightColor = m_cloudColor.lighter(105);
    highlightColor.setAlpha(60);
    gradient.setColorAt(0, highlightColor);
    gradient.setColorAt(1, Qt::transparent);
    
    painter->setBrush(gradient);
    painter->setPen(Qt::NoPen);
    
    // 在重叠的云朵上方添加高光区域
    painter->drawEllipse(QPointF(m_width * 0.4, m_height * 0.3), m_width * 0.3, m_height * 0.25);
    
    // 顶部圆形群
    qreal topRadius1 = m_height * 0.35;
    QRectF topCloud1(m_width * 0.35, m_height * 0.05, topRadius1 * 2, topRadius1 * 2);
    painter->drawEllipse(topCloud1);
    
    qreal topRadius2 = m_height * 0.3;
    QRectF topCloud2(m_width * 0.5, m_height * 0.02, topRadius2 * 2, topRadius2 * 2);
    painter->drawEllipse(topCloud2);
    
    // 底部圆形群（增加蓬松感）
    qreal bottomRadius1 = m_height * 0.25;
    QRectF bottomCloud1(m_width * 0.15, m_height * 0.6, bottomRadius1 * 2, bottomRadius1 * 2);
    painter->drawEllipse(bottomCloud1);
    
    qreal bottomRadius2 = m_height * 0.28;
    QRectF bottomCloud2(m_width * 0.55, m_height * 0.65, bottomRadius2 * 2, bottomRadius2 * 2);
    painter->drawEllipse(bottomCloud2);
    
    // 添加更多小的装饰圆形增加蓬松效果
    for (int i = 0; i < 5; ++i) {
        qreal smallRadius = m_height * (0.12 + (i % 3) * 0.03);
        qreal randomX = m_width * (0.1 + i * 0.18);
        qreal randomY = m_height * (0.4 + (i % 2) * 0.25);
        QRectF smallCloud(randomX, randomY, smallRadius * 2, smallRadius * 2);
        painter->drawEllipse(smallCloud);
    }
    
    // 添加中等大小的填充圆形
    for (int i = 0; i < 3; ++i) {
        qreal mediumRadius = m_height * 0.2;
        qreal posX = m_width * (0.2 + i * 0.3);
        qreal posY = m_height * (0.45 + (i % 2) * 0.1);
        QRectF mediumCloud(posX, posY, mediumRadius * 2, mediumRadius * 2);
        painter->drawEllipse(mediumCloud);
    }
}

void CloudEntity::drawStratusCloud(QPainter* painter)
{
    // 绘制梦幻层云（长条形，柔和渐变，营造浪漫氛围）
    painter->setBrush(QBrush(m_cloudColor));
    painter->setPen(QPen(Qt::transparent)); // 无边框更柔和
    
    // 创建柔和的渐变效果作为底色
    QLinearGradient gradient(0, 0, m_width, m_height);
    QColor baseColor = m_cloudColor;
    QColor edgeColor = baseColor;
    edgeColor.setAlpha(20);
    gradient.setColorAt(0.0, edgeColor);
    gradient.setColorAt(0.3, baseColor);
    gradient.setColorAt(0.7, baseColor);
    gradient.setColorAt(1.0, edgeColor);
    
    // 主体长椭圆（更大更柔和）
    QRectF mainCloud(m_width * 0.05, m_height * 0.25, m_width * 0.9, m_height * 0.5);
    painter->setBrush(gradient);
    painter->drawEllipse(mainCloud);
    
    painter->setBrush(QBrush(m_cloudColor));
    
    // 上层椭圆群 - 更加流畅的形状
    QRectF topCloud1(m_width * 0.08, m_height * 0.08, m_width * 0.75, m_height * 0.35);
    painter->drawEllipse(topCloud1);
    
    QRectF topCloud2(m_width * 0.22, m_height * 0.03, m_width * 0.55, m_height * 0.3);
    painter->drawEllipse(topCloud2);
    
    // 下层延伸群 - 更加丰满
    QRectF bottomCloud1(m_width * 0.06, m_height * 0.55, m_width * 0.85, m_height * 0.32);
    painter->drawEllipse(bottomCloud1);
    
    QRectF bottomCloud2(m_width * 0.12, m_height * 0.65, m_width * 0.7, m_height * 0.27);
    painter->drawEllipse(bottomCloud2);
    
    // 添加边缘的小椭圆增加柔和蓬松感
    QRectF leftEdge(m_width * 0.01, m_height * 0.33, m_width * 0.22, m_height * 0.35);
    painter->drawEllipse(leftEdge);
    
    QRectF rightEdge(m_width * 0.8, m_height * 0.38, m_width * 0.2, m_height * 0.28);
    painter->drawEllipse(rightEdge);
    
    // 中间层次的椭圆 - 增加层次感
    QRectF middleLayer1(m_width * 0.18, m_height * 0.28, m_width * 0.42, m_height * 0.22);
    painter->drawEllipse(middleLayer1);
    
    QRectF middleLayer2(m_width * 0.42, m_height * 0.32, m_width * 0.38, m_height * 0.2);
    painter->drawEllipse(middleLayer2);
    
    // 添加微妙的高光效果
    painter->setBrush(QBrush(QColor(255, 255, 255, 30)));
    painter->drawEllipse(QPointF(m_width * 0.35, m_height * 0.25), m_width * 0.25, m_height * 0.15);
}

void CloudEntity::drawCirrusCloud(QPainter* painter)
{
    // 绘制轻盈卷云（丝状，梦幻渐变，更加浪漫）
    // 使用渐变色彩增加梦幻感
    QColor baseColor = m_cloudColor;
    QColor edgeColor = QColor(255, 255, 255, 40);  // 边缘几乎透明
    QColor accentColor = m_cloudColor.lighter(110);
    accentColor.setAlpha(150);
    
    // 主要路径 - 使用更流畅的线条和渐变
    QPainterPath mainCloudPath;
    QPointF startPoint(m_width * 0.03, m_height * 0.5);
    mainCloudPath.moveTo(startPoint);
    
    // 创建更加优雅流畅的波浪状路径
    for (int i = 1; i <= 12; ++i) {  // 增加更多点使曲线更平滑
        qreal x = m_width * (0.03 + i * 0.08);
        qreal y = m_height * (0.5 + qSin(i * 0.5) * 0.28);  // 更大的振幅
        QPointF controlPoint1(x - m_width * 0.025, y - m_height * 0.09);
        QPointF controlPoint2(x + m_width * 0.025, y + m_height * 0.09);
        QPointF endPoint(x, y);
        
        mainCloudPath.cubicTo(controlPoint1, controlPoint2, endPoint);
    }
    
    // 创建渐变笔刷
    QLinearGradient gradient(0, 0, 0, m_height);
    gradient.setColorAt(0.0, edgeColor);
    gradient.setColorAt(0.3, baseColor);
    gradient.setColorAt(0.7, baseColor);
    gradient.setColorAt(1.0, edgeColor);
    
    // 绘制主路径 - 更宽，更柔和的边缘
    QPen mainCloudPen(gradient, m_height * 0.28);
    mainCloudPen.setCapStyle(Qt::RoundCap);
    painter->setPen(mainCloudPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(mainCloudPath);
    
    // 添加第二条主要丝线 - 更加浪漫的曲线
    QPainterPath secondPath;
    secondPath.moveTo(m_width * 0.08, m_height * 0.25);
    for (int i = 1; i <= 10; ++i) {  // 增加点数提高平滑度
        qreal x = m_width * (0.08 + i * 0.09);
        qreal y = m_height * (0.25 + qSin(i * 0.7 + 1.2) * 0.22);
        // 使用曲线而不是直线连接，使形状更柔和
        QPointF controlPoint(x - m_width * 0.04, y - m_height * 0.02);
        secondPath.quadTo(controlPoint, QPointF(x, y));
    }
    
    QPen secondPen(accentColor, m_height * 0.22);
    secondPen.setCapStyle(Qt::RoundCap);
    painter->setPen(secondPen);
    painter->drawPath(secondPath);
      // 添加第三条丝线 - 更加优雅的弯曲
    QPainterPath thirdPath;
    thirdPath.moveTo(m_width * 0.15, m_height * 0.7);
    for (int i = 1; i <= 7; ++i) {  // 增加一个点使曲线更平滑
        qreal x = m_width * (0.15 + i * 0.11);
        qreal y = m_height * (0.7 + qSin(i * 0.9 + 3) * 0.18);  // 微调波浪频率和幅度
        
        // 使用贝塞尔曲线代替直线，使形状更加优美流畅
        if (i > 1) {
            QPointF prevPoint = thirdPath.currentPosition();
            QPointF ctrlPoint((prevPoint.x() + x) / 2, prevPoint.y() - m_height * 0.05);
            thirdPath.quadTo(ctrlPoint, QPointF(x, y));
        } else {
            thirdPath.lineTo(x, y);
        }
    }
    
    // 使用轻微渐变增强层次感
    QLinearGradient thirdGradient(0, m_height * 0.7, m_width, m_height * 0.85);
    thirdGradient.setColorAt(0.0, QColor(baseColor.red(), baseColor.green(), baseColor.blue(), 100));
    thirdGradient.setColorAt(0.5, baseColor);
    thirdGradient.setColorAt(1.0, QColor(baseColor.red(), baseColor.green(), baseColor.blue(), 120));
    
    QPen thirdPen(thirdGradient, m_height * 0.18);  // 稍微增加宽度
    thirdPen.setCapStyle(Qt::RoundCap);
    painter->setPen(thirdPen);
    painter->drawPath(thirdPath);
    
    // 添加更多细丝线增加浪漫的复杂度和轻盈感
    for (int i = 0; i < 5; ++i) {  // 增加一条丝线
        QPainterPath threadPath;
        qreal startX = m_width * (0.08 + i * 0.19);  // 稍微调整分布
        qreal startY = m_height * (0.18 + i * 0.16); 
        threadPath.moveTo(startX, startY);
        
        for (int j = 1; j <= 6; ++j) {  // 增加一个点使曲线更平滑
            qreal x = startX + m_width * (j * 0.07);
            qreal y = startY + m_height * (qSin(j * 1.2 + i * 0.8) * 0.14);
            
            // 使用贝塞尔曲线使连接更优雅
            if (j > 1) {
                QPointF prevPoint = threadPath.currentPosition();
                QPointF ctrlPoint((prevPoint.x() + x) / 2, prevPoint.y() + (y - prevPoint.y()) * 0.3);
                threadPath.quadTo(ctrlPoint, QPointF(x, y));
            } else {
                threadPath.lineTo(x, y);
            }
        }
        
        // 为细丝线添加淡雅的透明度变化
        QColor threadColor = baseColor;
        threadColor.setAlpha(120 + i * 20);  // 不同的透明度增加层次感
        
        QPen threadPen(threadColor, m_height * (0.06 + i * 0.01));  // 粗细微微变化
        threadPen.setCapStyle(Qt::RoundCap);
        painter->setPen(threadPen);
        painter->drawPath(threadPath);
    }
    
    // 添加几个微妙的闪光点，增加浪漫的魔幻感
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor(255, 255, 255, 60)));
    
    for (int i = 0; i < 6; ++i) {
        qreal sparkleX = m_width * (0.2 + i * 0.15);
        qreal sparkleY = m_height * (0.3 + qSin(i * 1.5) * 0.3);
        qreal size = m_height * (0.02 + (i % 3) * 0.01);
        painter->drawEllipse(QPointF(sparkleX, sparkleY), size, size);
    }
}

#include "cloudentity.moc"
