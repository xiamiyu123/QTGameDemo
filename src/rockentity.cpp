#include "rockentity.h"
#include <QRandomGenerator>
#include "debuglogger.h"

RockEntity::RockEntity(qreal width, qreal height, QGraphicsItem* parent)
    : BasePhysicsEntity(width, height, parent), m_textureLoaded(false)
{
    setZValue(-2);
    setBrush(QBrush(Qt::black));
    setPen(QPen(Qt::black, 2));
    setEntityType(EntityType::Obstacle);

    // 固定石头，不受物理影响
    m_physicsComponent->setGravity(0);
    m_physicsComponent->setFrictionFactor(0);

    // 加载石头贴图
    loadRockTexture();
}

void RockEntity::loadRockTexture()
{
    // 随机选择石头贴图 png1.png 或 png2.png
    int randomChoice = QRandomGenerator::global()->bounded(1, 3); // 生成1或2
    QString filename = QString("env_rocks_%1.png").arg(QString::number(randomChoice));
    QString basePath = ":/resource/images/obstacle/stone/";
    QString fullPath = basePath + filename;

    DEBUG_LOG(QString("Attempting to load rock texture: %1").arg(fullPath));

    // 尝试加载贴图
    QPixmap pixmap(fullPath);
    if (!pixmap.isNull()) {
        // 获取当前石头的尺寸
        QRectF bounds = rect();

        // 缩放贴图到石头尺寸，保持宽高比
        m_rockTexture = pixmap.scaled(
            static_cast<int>(bounds.width()),
            static_cast<int>(bounds.height()),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );

        m_textureLoaded = true;
        DEBUG_LOG(QString("Successfully loaded rock texture: %1 (size: %2x%3)")
         .arg(filename)
         .arg(QString::number(m_rockTexture.width()))
         .arg(QString::number(m_rockTexture.height())));
    } else {
        DEBUG_LOG(QString("Failed to load rock texture: %1, trying alternative").arg(fullPath));

        // 如果加载失败，尝试加载另一个
        int alternativeChoice = (randomChoice == 1) ? 2 : 1;
        QString alternativeFilename = QString("png%1.png").arg(QString::number(alternativeChoice));
        QString alternativeFullPath = basePath + alternativeFilename;

        QPixmap alternativePixmap(alternativeFullPath);
        if (!alternativePixmap.isNull()) {
            QRectF bounds = rect();
            m_rockTexture = alternativePixmap.scaled(
                static_cast<int>(bounds.width()),
                static_cast<int>(bounds.height()),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            );
            m_textureLoaded = true;
            DEBUG_LOG(QString("Successfully loaded alternative rock texture: %1").arg(alternativeFilename));
        } else {
            m_textureLoaded = false;
            DEBUG_LOG("Failed to load both rock textures, using default black appearance");
        }
    }
}

void RockEntity::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (m_textureLoaded && !m_rockTexture.isNull()) {
        // 绘制石头贴图
        QRectF bounds = rect();
        painter->drawPixmap(bounds.toRect(), m_rockTexture);

        // 可选：绘制调试边框（可以注释掉）
        // #ifdef DEBUG_ROCK_BOUNDS
        // QPen debugPen(Qt::red, 1);
        // painter->setPen(debugPen);
        // painter->drawRect(bounds);
        // #endif

    } else {
        // 如果贴图未加载，回退到默认的黑色石头绘制
        QGraphicsRectItem::paint(painter, option, widget);

        // 可选：在默认石头上绘制一些纹理线条
        QRectF bounds = rect();
        QPen texturePen(Qt::darkGray, 1);
        painter->setPen(texturePen);

        // 绘制一些随机的纹理线条来模拟石头表面
        int lineCount = 3;
        for (int i = 0; i < lineCount; ++i) {
            qreal startX = bounds.left() + bounds.width() * 0.2;
            qreal endX = bounds.right() - bounds.width() * 0.2;
            qreal y = bounds.top() + bounds.height() * (0.3 + i * 0.2);
            painter->drawLine(QPointF(startX, y), QPointF(endX, y));
        }
    }
}