// penguinnpc.cpp - 实现贴图功能
#include "penguinnpc.h"
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QColor>
#include "debuglogger.h"

PenguinNPC::PenguinNPC(QGraphicsItem *parent)
    : GroundNPC(parent)
    , m_currentFrame(0)
    , m_textureLoaded(false)
{
    setRect(0, 0, PENGUIN_WIDTH, PENGUIN_HEIGHT);
    setBrush(QBrush(QColor(50, 50, 50)));
    setPen(QPen(Qt::white, 2));
    setMovementSpeed(PENGUIN_SPEED);
    
    // 加载动画帧
    loadAnimationFrames();
    
    // 设置动画定时器
    connect(&m_animationTimer, &QTimer::timeout, this, &PenguinNPC::updateAnimation);
    m_animationTimer.start(150); // 每150ms切换一帧
    
    DEBUG_LOG("PenguinNPC created");
}

void PenguinNPC::loadAnimationFrames()
{
    m_animationFrames.clear();
    
    // 加载企鹅跑步动画 penguin1.png 到 penguin8.png
    QString basePath = ":/resource/images/npcs/penguin/penguin_running/";
    
    for (int i = 1; i <= 8; ++i) {
        QString filename = QString("penguin%1.png").arg(i);
        QString fullPath = basePath + filename;
        
        QPixmap pixmap(fullPath);
        if (!pixmap.isNull()) {
            // 缩放到合适大小
            QPixmap scaledPixmap = pixmap.scaled(
                static_cast<int>(PENGUIN_WIDTH),
                static_cast<int>(PENGUIN_HEIGHT),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            );
            m_animationFrames.append(scaledPixmap);
            DEBUG_LOG(QString("Loaded penguin frame: %1").arg(filename));
        } else {
            DEBUG_LOG(QString("Failed to load penguin frame: %1").arg(fullPath));
        }
    }
    
    if (!m_animationFrames.isEmpty()) {
        m_textureLoaded = true;
        m_currentFrame = 0;
        DEBUG_LOG(QString("Successfully loaded %1 penguin animation frames").arg(m_animationFrames.size()));
    } else {
        m_textureLoaded = false;
        DEBUG_LOG("Failed to load penguin animation frames, using default appearance");
    }
}

void PenguinNPC::updateAnimation()
{
    if (m_textureLoaded && !m_animationFrames.isEmpty()) {
        m_currentFrame = (m_currentFrame + 1) % m_animationFrames.size();
    }
}

void PenguinNPC::initializeNPC()
{
    GroundNPC::initializeNPC();
    setMovementSpeed(PENGUIN_SPEED);
    setActive(false);
    DEBUG_LOG("PenguinNPC initialized");
}

void PenguinNPC::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (m_textureLoaded && !m_animationFrames.isEmpty() && 
        m_currentFrame >= 0 && m_currentFrame < m_animationFrames.size()) {
        
        // 绘制当前动画帧
        QRectF rect = boundingRect();
        const QPixmap& currentPixmap = m_animationFrames[m_currentFrame];
        painter->drawPixmap(rect.toRect(), currentPixmap);
        
    } else {
        // 如果贴图未加载，使用原来的绘制方法
        QRectF rect = boundingRect();
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
}

QRectF PenguinNPC::boundingRect() const
{
    return QRectF(0, 0, PENGUIN_WIDTH, PENGUIN_HEIGHT);
}