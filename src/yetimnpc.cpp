// yetimnpc.cpp - 实现贴图功能
#include "yetimnpc.h"
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QColor>
#include "debuglogger.h"

YetiNPC::YetiNPC(QGraphicsItem *parent)
    : GroundNPC(parent)
    , m_currentFrame(0)
    , m_textureLoaded(false)
{
    setRect(0, 0, YETI_WIDTH, YETI_HEIGHT);
    setBrush(QBrush(QColor(230, 240, 255)));
    setPen(QPen(QColor(200, 220, 240), 3));
    setMovementSpeed(YETI_SPEED);
    
    // 加载动画帧
    loadAnimationFrames();
    
    // 设置动画定时器
    connect(&m_animationTimer, &QTimer::timeout, this, &YetiNPC::updateAnimation);
    m_animationTimer.start(50);// 每50毫秒更新一帧，约20FPS

    DEBUG_LOG("YetiNPC created");
}

void YetiNPC::loadAnimationFrames()
{
    m_animationFrames.clear();
    
    // 加载雪怪跑步动画 yeti1.png 到 yeti16.png
    QString basePath = ":/resource/images/npcs/yeti/yeti_running/";
    
    for (int i = 1; i <= 16; ++i) {
        QString filename = QString("yeti%1.png").arg(i);
        QString fullPath = basePath + filename;
        
        QPixmap pixmap(fullPath);
        if (!pixmap.isNull()) {
            // 缩放到合适大小
            QPixmap scaledPixmap = pixmap.scaled(
                static_cast<int>(YETI_WIDTH),
                static_cast<int>(YETI_HEIGHT),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            );
            m_animationFrames.append(scaledPixmap);
            DEBUG_LOG(QString("Loaded yeti frame: %1").arg(filename));
        } else {
            DEBUG_LOG(QString("Failed to load yeti frame: %1").arg(fullPath));
        }
    }
    
    if (!m_animationFrames.isEmpty()) {
        m_textureLoaded = true;
        m_currentFrame = 0;
        DEBUG_LOG(QString("Successfully loaded %1 yeti animation frames").arg(m_animationFrames.size()));
    } else {
        m_textureLoaded = false;
        DEBUG_LOG("Failed to load yeti animation frames, using default appearance");
    }
}

void YetiNPC::updateAnimation()
{
    if (m_textureLoaded && !m_animationFrames.isEmpty()) {
        m_currentFrame = (m_currentFrame + 1) % m_animationFrames.size();
    }
}

void YetiNPC::initializeNPC()
{
    GroundNPC::initializeNPC();
    setMovementSpeed(YETI_SPEED);
    setActive(false);
    getPhysicsComponent()->setGravity(0);
    DEBUG_LOG("YetiNPC initialized");
}

void YetiNPC::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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
        painter->setBrush(QBrush(QColor(230, 240, 255)));
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
}

QRectF YetiNPC::boundingRect() const
{
    return QRectF(-10, 0, YETI_WIDTH + 20, YETI_HEIGHT);
}

