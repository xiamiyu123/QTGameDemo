#include "treeentity.h"
#include <QBrush>
#include <QPen>
#include <QPolygonF>
#include <QtMath>

TreeEntity::TreeEntity(qreal width, qreal height, QGraphicsItem* parent)
    : QGraphicsRectItem(0, 0, width, height, parent)
    , m_width(width)
    , m_height(height)
{    // 随机选择树木类型 (0=雪覆松树, 1=雪中枯树) - 删除冰霜树，保持浪漫雪原风格
    m_treeType = QRandomGenerator::global()->bounded(2);
    
    // 设置雪原专用的精美颜色方案
    switch (m_treeType) {    case 0: // 雪覆松树 - 厚重积雪的冬季松树，增加浪漫的粉色调
        m_trunkColor = QColor(78, 53, 42);        // 深褐色树干
        m_leavesColor = QColor(34, 59, 45);       // 深森林绿
        m_snowColor = QColor(255, 248, 250, 240); // 带微弱粉色调的雪白
        break;
    case 1: // 雪中枯树 - 优雅的冬季落叶树，增强温暖的浪漫感
        m_trunkColor = QColor(95, 87, 79);        // 暖灰褐色
        m_leavesColor = QColor(180, 142, 108);    // 更暖的枯黄色，增加温馨感
        m_snowColor = QColor(255, 250, 250, 220); // 温暖的雪白色调
        break;
    }
    
    // 设置边界框透明
    setBrush(QBrush(Qt::transparent));
    setPen(QPen(Qt::transparent));
}

void TreeEntity::setPosition(const QPointF& position)
{
    setPos(position.x() - m_width / 2, position.y() - m_height);
}

void TreeEntity::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    
    painter->setRenderHint(QPainter::Antialiasing);    // 根据树木类型绘制不同形状 - 只保留两种浪漫树型
    switch (m_treeType) {
    case 0:
        drawSnowPine(painter);
        break;
    case 1:
        drawDeadTree(painter);
        break;
    }
}

void TreeEntity::drawSnowPine(QPainter* painter)
{    // 绘制雪覆松树的厚实树干 - 更优雅的弧度
    qreal trunkWidth = m_width * 0.22; // 稍微加粗树干
    qreal trunkHeight = m_height * 0.4;
    qreal trunkX = m_width / 2 - trunkWidth / 2;
    qreal trunkY = m_height - trunkHeight;
    
    // 树干主体 - 更优美的弧度和形状，增强浪漫感
    QPainterPath trunkPath;
    trunkPath.moveTo(trunkX, m_height);
    // 左侧弧线更加优雅
    trunkPath.cubicTo(trunkX - trunkWidth * 0.15, m_height - trunkHeight * 0.3,
                      trunkX + trunkWidth * 0.08, trunkY + trunkHeight * 0.25,
                      trunkX, trunkY);
    trunkPath.lineTo(trunkX + trunkWidth, trunkY);
    // 右侧弧线呼应左侧
    trunkPath.cubicTo(trunkX + trunkWidth - trunkWidth * 0.08, trunkY + trunkHeight * 0.25,
                      trunkX + trunkWidth + trunkWidth * 0.15, m_height - trunkHeight * 0.3,
                      trunkX + trunkWidth, m_height);
    trunkPath.closeSubpath();
    
    // 绘制树干
    painter->setBrush(QBrush(m_trunkColor));
    painter->setPen(QPen(m_trunkColor.darker(140), 2));
    painter->drawPath(trunkPath);
    
    // 树干纹理 - 树皮裂纹
    painter->setPen(QPen(m_trunkColor.darker(180), 1));
    for (int i = 1; i <= 4; ++i) {
        qreal lineY = trunkY + trunkHeight * (i * 0.22);
        painter->drawLine(trunkX + 2, lineY, trunkX + trunkWidth - 2, lineY);
    }
    
    // 绘制松树的层次针叶 - 5层渐变效果
    painter->setBrush(QBrush(m_leavesColor));
    painter->setPen(QPen(m_leavesColor.darker(120), 1.5));
    
    // 底层 - 最大最蓬松
    qreal layer1Y = m_height * 0.55;
    qreal layer1Width = m_width * 0.9;
    qreal layer1Height = m_height * 0.25;
    
    QPainterPath layer1;
    layer1.moveTo(m_width / 2, layer1Y - layer1Height * 0.3);
    // 左侧蓬松边缘
    layer1.cubicTo(m_width / 2 - layer1Width * 0.2, layer1Y - layer1Height * 0.1,
                   m_width / 2 - layer1Width * 0.4, layer1Y + layer1Height * 0.2,
                   m_width / 2 - layer1Width * 0.45, layer1Y + layer1Height * 0.5);
    layer1.cubicTo(m_width / 2 - layer1Width * 0.35, layer1Y + layer1Height * 0.7,
                   m_width / 2 - layer1Width * 0.15, layer1Y + layer1Height * 0.8,
                   m_width / 2, layer1Y + layer1Height * 0.7);
    // 右侧蓬松边缘
    layer1.cubicTo(m_width / 2 + layer1Width * 0.15, layer1Y + layer1Height * 0.8,
                   m_width / 2 + layer1Width * 0.35, layer1Y + layer1Height * 0.7,
                   m_width / 2 + layer1Width * 0.45, layer1Y + layer1Height * 0.5);
    layer1.cubicTo(m_width / 2 + layer1Width * 0.4, layer1Y + layer1Height * 0.2,
                   m_width / 2 + layer1Width * 0.2, layer1Y - layer1Height * 0.1,
                   m_width / 2, layer1Y - layer1Height * 0.3);
    layer1.closeSubpath();
    painter->drawPath(layer1);
    
    // 第二层
    qreal layer2Y = m_height * 0.45;
    qreal layer2Width = m_width * 0.75;
    qreal layer2Height = m_height * 0.2;
    
    QPainterPath layer2;
    layer2.moveTo(m_width / 2, layer2Y - layer2Height * 0.4);
    layer2.cubicTo(m_width / 2 - layer2Width * 0.25, layer2Y,
                   m_width / 2 - layer2Width * 0.35, layer2Y + layer2Height * 0.6,
                   m_width / 2, layer2Y + layer2Height * 0.6);
    layer2.cubicTo(m_width / 2 + layer2Width * 0.35, layer2Y + layer2Height * 0.6,
                   m_width / 2 + layer2Width * 0.25, layer2Y,
                   m_width / 2, layer2Y - layer2Height * 0.4);
    layer2.closeSubpath();
    painter->drawPath(layer2);
    
    // 第三层
    qreal layer3Y = m_height * 0.35;
    qreal layer3Width = m_width * 0.6;
    qreal layer3Height = m_height * 0.15;
    
    QPainterPath layer3;
    layer3.moveTo(m_width / 2, layer3Y - layer3Height * 0.5);
    layer3.cubicTo(m_width / 2 - layer3Width * 0.3, layer3Y,
                   m_width / 2 - layer3Width * 0.35, layer3Y + layer3Height * 0.7,
                   m_width / 2, layer3Y + layer3Height * 0.5);
    layer3.cubicTo(m_width / 2 + layer3Width * 0.35, layer3Y + layer3Height * 0.7,
                   m_width / 2 + layer3Width * 0.3, layer3Y,
                   m_width / 2, layer3Y - layer3Height * 0.5);
    layer3.closeSubpath();
    painter->drawPath(layer3);
    
    // 第四层
    qreal layer4Y = m_height * 0.25;
    qreal layer4Width = m_width * 0.45;
    qreal layer4Height = m_height * 0.12;
    
    QPainterPath layer4;
    layer4.moveTo(m_width / 2, layer4Y - layer4Height * 0.6);
    layer4.cubicTo(m_width / 2 - layer4Width * 0.35, layer4Y,
                   m_width / 2 - layer4Width * 0.4, layer4Y + layer4Height * 0.8,
                   m_width / 2, layer4Y + layer4Height * 0.4);
    layer4.cubicTo(m_width / 2 + layer4Width * 0.4, layer4Y + layer4Height * 0.8,
                   m_width / 2 + layer4Width * 0.35, layer4Y,
                   m_width / 2, layer4Y - layer4Height * 0.6);
    layer4.closeSubpath();
    painter->drawPath(layer4);
    
    // 顶层 - 小巧精致
    qreal layer5Y = m_height * 0.15;
    qreal layer5Width = m_width * 0.3;
    qreal layer5Height = m_height * 0.1;
    
    QPainterPath layer5;
    layer5.moveTo(m_width / 2, layer5Y - layer5Height * 0.8);
    layer5.cubicTo(m_width / 2 - layer5Width * 0.4, layer5Y,
                   m_width / 2 - layer5Width * 0.3, layer5Y + layer5Height * 0.6,
                   m_width / 2, layer5Y + layer5Height * 0.2);
    layer5.cubicTo(m_width / 2 + layer5Width * 0.3, layer5Y + layer5Height * 0.6,
                   m_width / 2 + layer5Width * 0.4, layer5Y,
                   m_width / 2, layer5Y - layer5Height * 0.8);
    layer5.closeSubpath();
    painter->drawPath(layer5);
    
    // 添加厚重的积雪效果
    painter->setBrush(QBrush(m_snowColor));
    painter->setPen(QPen(m_snowColor.darker(110), 1));
    
    // 每层针叶上的积雪 - 不规则形状
    QVector<qreal> layerYs = {layer1Y, layer2Y, layer3Y, layer4Y, layer5Y};
    QVector<qreal> layerWidths = {layer1Width, layer2Width, layer3Width, layer4Width, layer5Width};
    
    for (int i = 0; i < layerYs.size(); ++i) {
        qreal snowY = layerYs[i] - (i == 0 ? layer1Height * 0.3 : (i == 1 ? layer2Height * 0.4 : 
                                           (i == 2 ? layer3Height * 0.5 : (i == 3 ? layer4Height * 0.6 : layer5Height * 0.8))));
        qreal snowWidth = layerWidths[i] * 0.8;
        qreal snowHeight = m_height * (0.04 + i * 0.008);
        
        QPainterPath snow;
        snow.moveTo(m_width / 2, snowY);
        snow.cubicTo(m_width / 2 - snowWidth * 0.25, snowY + snowHeight * 0.5,
                     m_width / 2 - snowWidth * 0.35, snowY + snowHeight,
                     m_width / 2, snowY + snowHeight * 0.8);
        snow.cubicTo(m_width / 2 + snowWidth * 0.35, snowY + snowHeight,
                     m_width / 2 + snowWidth * 0.25, snowY + snowHeight * 0.5,
                     m_width / 2, snowY);
        snow.closeSubpath();
        painter->drawPath(snow);
      // 在雪层上添加浪漫的雪花装饰和星星点点的光芒
    painter->setBrush(QBrush(Qt::white));
    painter->setPen(Qt::NoPen);
    for (int j = 0; j < 5; ++j) { // 增加雪花数量
        qreal snowflakeX = m_width / 2 + (j - 2) * snowWidth * 0.15;
        qreal snowflakeY = snowY + snowHeight * 0.3;
        painter->drawEllipse(QPointF(snowflakeX, snowflakeY), 1.5, 1.5);
        
        // 添加微小的光点效果
        if (j % 2 == 0) {
            painter->setBrush(QBrush(QColor(255, 255, 240, 150))); // 淡黄光芒
            painter->drawEllipse(QPointF(snowflakeX, snowflakeY), 0.8, 0.8);
            painter->setBrush(QBrush(Qt::white));
        }
    }
    }
      // 树干上的积雪 - 更丰富更浪漫的积雪效果
    painter->setBrush(QBrush(m_snowColor));
    painter->setPen(QPen(m_snowColor.darker(105), 0.8));
    
    // 主要的积雪层 - 更加蓬松
    QRectF trunkSnow(trunkX - 4, trunkY, trunkWidth + 8, trunkHeight * 0.22);
    painter->drawRoundedRect(trunkSnow, 5, 5);
    
    // 添加一些垂落的小雪块，增加立体感
    QPainterPath snowDrip;
    snowDrip.addRoundedRect(trunkX + trunkWidth * 0.3, trunkY + trunkHeight * 0.18,
                           trunkWidth * 0.2, trunkHeight * 0.1, 3, 3);
    painter->drawPath(snowDrip);
    
    // 添加一些反光点，让雪看起来闪闪发光
    painter->setBrush(QBrush(QColor(255, 255, 255, 180)));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(trunkX + trunkWidth * 0.25, trunkY + trunkHeight * 0.1), 2, 1.5);
    painter->drawEllipse(QPointF(trunkX + trunkWidth * 0.7, trunkY + trunkHeight * 0.05), 1.5, 1);
}

void TreeEntity::drawDeadTree(QPainter* painter)
{
    // 绘制优雅的雪中枯树主干
    qreal trunkWidth = m_width * 0.22;
    qreal trunkHeight = m_height * 0.65;
    qreal trunkX = m_width / 2 - trunkWidth / 2;
    qreal trunkY = m_height - trunkHeight;
    
    // 主干 - 自然弯曲形状
    QPainterPath trunkPath;
    trunkPath.moveTo(trunkX, m_height);
    trunkPath.cubicTo(trunkX - trunkWidth * 0.05, m_height - trunkHeight * 0.4,
                      trunkX + trunkWidth * 0.08, trunkY + trunkHeight * 0.3,
                      trunkX + trunkWidth * 0.1, trunkY);
    trunkPath.lineTo(trunkX + trunkWidth - trunkWidth * 0.1, trunkY);
    trunkPath.cubicTo(trunkX + trunkWidth - trunkWidth * 0.08, trunkY + trunkHeight * 0.3,
                      trunkX + trunkWidth + trunkWidth * 0.05, m_height - trunkHeight * 0.4,
                      trunkX + trunkWidth, m_height);
    trunkPath.closeSubpath();
    
    painter->setBrush(QBrush(m_trunkColor));
    painter->setPen(QPen(m_trunkColor.darker(140), 2));
    painter->drawPath(trunkPath);
    
    // 细致的树皮纹理
    painter->setPen(QPen(m_trunkColor.darker(160), 1));
    for (int i = 1; i <= 5; ++i) {
        qreal lineY = trunkY + trunkHeight * (i * 0.18);
        QPointF start(trunkX + 2 + (i % 2) * 3, lineY);
        QPointF end(trunkX + trunkWidth - 2 - (i % 2) * 3, lineY);
        painter->drawLine(start, end);
    }
    
    // 垂直纹理和节疤
    painter->setPen(QPen(m_trunkColor.darker(180), 1));
    painter->drawLine(trunkX + trunkWidth * 0.3, trunkY + trunkHeight * 0.2,
                     trunkX + trunkWidth * 0.3, trunkY + trunkHeight * 0.8);
    painter->drawLine(trunkX + trunkWidth * 0.7, trunkY + trunkHeight * 0.3,
                     trunkX + trunkWidth * 0.7, trunkY + trunkHeight * 0.9);
    
    // 绘制优雅的主分支系统
    qreal centerX = m_width / 2;
    qreal mainBranchY = trunkY + trunkHeight * 0.25;
    
    // 左侧主分支 - 弯曲上升
    painter->setPen(QPen(m_trunkColor.darker(120), 6));
    painter->setBrush(Qt::NoBrush);
    
    QPainterPath leftBranch;
    leftBranch.moveTo(centerX, mainBranchY);
    leftBranch.cubicTo(centerX - m_width * 0.15, mainBranchY - m_height * 0.05,
                       centerX - m_width * 0.25, mainBranchY - m_height * 0.12,
                       centerX - m_width * 0.35, mainBranchY - m_height * 0.18);
    painter->drawPath(leftBranch);
    
    // 右侧主分支 - 略微不对称
    QPainterPath rightBranch;
    rightBranch.moveTo(centerX, mainBranchY);
    rightBranch.cubicTo(centerX + m_width * 0.12, mainBranchY - m_height * 0.03,
                        centerX + m_width * 0.22, mainBranchY - m_height * 0.1,
                        centerX + m_width * 0.3, mainBranchY - m_height * 0.15);
    painter->drawPath(rightBranch);
    
    // 细分支系统
    painter->setPen(QPen(m_trunkColor.darker(110), 4));
    
    // 左侧分支的子分支
    QPointF leftEnd(centerX - m_width * 0.35, mainBranchY - m_height * 0.18);
    QPainterPath leftSub1;
    leftSub1.moveTo(leftEnd);
    leftSub1.cubicTo(leftEnd.x() - m_width * 0.08, leftEnd.y() - m_height * 0.06,
                     leftEnd.x() - m_width * 0.12, leftEnd.y() - m_height * 0.1,
                     leftEnd.x() - m_width * 0.15, leftEnd.y() - m_height * 0.12);
    painter->drawPath(leftSub1);
    
    QPainterPath leftSub2;
    leftSub2.moveTo(leftEnd);
    leftSub2.cubicTo(leftEnd.x() - m_width * 0.05, leftEnd.y() - m_height * 0.08,
                     leftEnd.x() - m_width * 0.08, leftEnd.y() - m_height * 0.14,
                     leftEnd.x() - m_width * 0.1, leftEnd.y() - m_height * 0.18);
    painter->drawPath(leftSub2);
    
    // 右侧分支的子分支
    QPointF rightEnd(centerX + m_width * 0.3, mainBranchY - m_height * 0.15);
    QPainterPath rightSub1;
    rightSub1.moveTo(rightEnd);
    rightSub1.cubicTo(rightEnd.x() + m_width * 0.06, rightEnd.y() - m_height * 0.05,
                      rightEnd.x() + m_width * 0.1, rightEnd.y() - m_height * 0.08,
                      rightEnd.x() + m_width * 0.12, rightEnd.y() - m_height * 0.12);
    painter->drawPath(rightSub1);
    
    QPainterPath rightSub2;
    rightSub2.moveTo(rightEnd);
    rightSub2.cubicTo(rightEnd.x() + m_width * 0.04, rightEnd.y() - m_height * 0.07,
                      rightEnd.x() + m_width * 0.07, rightEnd.y() - m_height * 0.12,
                      rightEnd.x() + m_width * 0.08, rightEnd.y() - m_height * 0.16);
    painter->drawPath(rightSub2);
    
    // 顶部细枝
    qreal topBranchY = trunkY + trunkHeight * 0.15;
    painter->setPen(QPen(m_trunkColor.darker(115), 3));
    
    QPainterPath topLeft;
    topLeft.moveTo(centerX, topBranchY);
    topLeft.cubicTo(centerX - m_width * 0.08, topBranchY - m_height * 0.04,
                    centerX - m_width * 0.12, topBranchY - m_height * 0.08,
                    centerX - m_width * 0.15, topBranchY - m_height * 0.1);
    painter->drawPath(topLeft);
    
    QPainterPath topRight;
    topRight.moveTo(centerX, topBranchY);
    topRight.cubicTo(centerX + m_width * 0.1, topBranchY - m_height * 0.03,
                     centerX + m_width * 0.15, topBranchY - m_height * 0.07,
                     centerX + m_width * 0.18, topBranchY - m_height * 0.1);
    painter->drawPath(topRight);
      // 稀疏但精美的枯叶簇 - 增加更多浪漫的叶子
    painter->setBrush(QBrush(m_leavesColor));
    painter->setPen(QPen(m_leavesColor.darker(130), 1));
    
    // 不规则的叶子簇 - 更自然更浪漫的形状
    QPointF leftSubEnd(leftEnd.x() - m_width * 0.15, leftEnd.y() - m_height * 0.12);
    QPainterPath leaf1;
    leaf1.addEllipse(leftSubEnd.x() - 10, leftSubEnd.y() - 8, 20, 16); // 稍大的叶子簇
    painter->drawPath(leaf1);
    
    QPointF rightSubEnd(rightEnd.x() + m_width * 0.12, rightEnd.y() - m_height * 0.12);
    QPainterPath leaf2;
    leaf2.addEllipse(rightSubEnd.x() - 8, rightSubEnd.y() - 10, 16, 20);
    painter->drawPath(leaf2);
    
    // 顶部小叶簇
    QPointF topEnd(centerX - m_width * 0.15, topBranchY - m_height * 0.1);
    QPainterPath leaf3;
    leaf3.addEllipse(topEnd.x() - 6, topEnd.y() - 6, 12, 12);
    painter->drawPath(leaf3);
    
    // 添加额外的小叶子簇，营造更丰富的效果
    QPointF rightTopEnd(centerX + m_width * 0.18, topBranchY - m_height * 0.1);
    QPainterPath leaf4;
    leaf4.addEllipse(rightTopEnd.x() - 4, rightTopEnd.y() - 5, 8, 10);
    painter->drawPath(leaf4);
      // 精美浪漫的积雪效果
    painter->setBrush(QBrush(m_snowColor));
    painter->setPen(QPen(m_snowColor.darker(105), 0.5)); // 微妙的边缘
    
    // 主干顶部的厚雪 - 更加蓬松饱满
    QPainterPath trunkSnow;
    trunkSnow.addRoundedRect(trunkX - 5, trunkY, trunkWidth + 10, trunkHeight * 0.22, 6, 6);
    painter->drawPath(trunkSnow);
    
    // 主分支上的雪堆 - 更加明显立体
    QPainterPath leftBranchSnow;
    leftBranchSnow.addEllipse(leftEnd.x() - 10, leftEnd.y() - 5, 20, 10);
    painter->drawPath(leftBranchSnow);
    
    QPainterPath rightBranchSnow;
    rightBranchSnow.addEllipse(rightEnd.x() - 8, rightEnd.y() - 4, 16, 8);
    painter->drawPath(rightBranchSnow);
    
    // 顶部分支的小雪堆
    QPainterPath topSnow;
    topSnow.addEllipse(topEnd.x() - 6, topEnd.y() - 3, 10, 5);
    painter->drawPath(topSnow);
    
    // 添加小的积雪点缀
    QPainterPath smallSnow;
    smallSnow.addEllipse(centerX - m_width * 0.05, topBranchY, 4, 2);
    painter->drawPath(smallSnow);
    
    // 雪的反光效果 - 营造梦幻感
    painter->setBrush(QBrush(QColor(255, 255, 255, 160)));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(leftEnd.x() - 5, leftEnd.y() - 2), 2, 1);
    painter->drawEllipse(QPointF(rightEnd.x() + 3, rightEnd.y() - 1), 1.5, 0.8);
    painter->drawEllipse(QPointF(topEnd.x() - 2, topEnd.y() - 1), 1.2, 0.6);
      // 为叶子簇添加更浪漫的雪花点缀和星光效果
    painter->setBrush(QBrush(Qt::white));
    painter->setPen(Qt::NoPen);
    
    // 在叶子簇上添加更多精致的雪花
    QVector<QPointF> leafCenters = {leftSubEnd, rightSubEnd, topEnd, rightTopEnd};
    for (const auto& center : leafCenters) {
        for (int i = 0; i < 3; ++i) {  // 增加雪花数量
            qreal offsetX = (i - 1.0) * 4;
            qreal offsetY = (i - 1.0) * 3;
            
            // 交替添加不同大小的雪花，营造立体感
            qreal size = (i % 2 == 0) ? 1.5 : 0.8;
            painter->drawEllipse(QPointF(center.x() + offsetX, center.y() + offsetY), size, size);
            
            // 为部分雪花添加轻微的金色/银色光晕，增强浪漫氛围
            if (i == 1) {
                painter->setBrush(QBrush(QColor(255, 250, 205, 120))); // 淡金色光晕
                painter->drawEllipse(QPointF(center.x() + offsetX, center.y() + offsetY), size+1.5, size+1.5);
                painter->setBrush(QBrush(Qt::white));
            }
        }
    }
}
