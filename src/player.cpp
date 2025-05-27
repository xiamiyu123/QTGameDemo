#include "player.h"
#include <QBrush>
#include <QPen>
#include "debuglogger.h"
#include <qpainter.h>

#include "terraingenerator.h"

// 定义静态常量
const qreal Player::MAX_LANDING_ANGLE_DEVIATION = 45.0; // 45度最大偏差

// 辅助函数：将角度归一化到[-180, 180]范围
static qreal normalizeAngle(qreal angle) {
    angle = fmod(angle, 360.0);
    if (angle > 180.0) {
        angle -= 360.0;
    } else if (angle <= -180.0) {
        angle += 360.0;
    }
    return angle;
}

Player::Player(QGraphicsItem *parent)
    : BasePhysicsEntity(30, 30, parent),
      is_fallen(false),
      keyLeft(false),
      keySpace(false),
      keyRight(false),
      m_moveSpeed(500),
      rotateSpeed(3),
      m_jumpForce(-300),
      m_takeoffRotation(0.0),
      m_flipRotation(0.0),
      m_cumulativeRotation(0.0),
      m_lastFrameRotation(0.0),
      m_imageScaleFactor(1) { // 添加图像缩放因子

    setZValue(-2);

    // 设置玩家外观
    setBrush(QBrush(Qt::red));
    setPen(QPen(Qt::black, 2));

    // 设置实体类型
    setEntityType(EntityType::Player);

    // 允许接收键盘焦点
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFocus();

    // 初始化摔倒恢复计时器
    connect(&m_fallRecoveryTimer, &QTimer::timeout, this, &Player::onFallRecoveryTimeout);
    m_fallRecoveryTimer.setSingleShot(true);
    loadAnimationFrames();

    // 设置动画定时器
    connect(&m_animationTimer, &QTimer::timeout, this, &Player::updateAnimation);
    m_animationTimer.start(100); // 每100毫秒更新一帧，约10FPS
}

Player::~Player() {
    // 父类析构函数会处理注销和组件删除
}
void Player::loadAnimationFrames() {
    m_animationFrames.clear();

    // 尝试加载png1到png38
    QString basePath = ":/resource/images/player/";

    for (int i = 1; i <= 38; ++i) {
        QString filename = QString("image%1.png").arg(QString::number(i));
        QString fullPath = basePath + filename;        QPixmap pixmap(fullPath);
        if (!pixmap.isNull()) {
            // 保存原始图像，不进行缩放，在绘制时再进行高质量缩放
            m_animationFrames.append(pixmap);
            DEBUG_LOG(QString("Loaded animation frame: %1").arg(filename));
        } else {
            DEBUG_LOG(QString("Failed to load animation frame: %1").arg(fullPath));
        }
    }

    if (!m_animationFrames.isEmpty()) {
        m_animationLoaded = true;
        m_currentFrame = 0;
        DEBUG_LOG(QString("Animation loaded successfully with %1 frames").arg(QString::number(m_animationFrames.size())));
    } else {
        DEBUG_LOG("Failed to load any animation frames, using default appearance");
        m_animationLoaded = false;
    }
}

int Player::getCurrentAnimationRange() const {
    if (is_fallen) {
        // 跳跃摔倒再站起：image16-25 (索引15-24)
        return 15 + (m_currentFrame % 10); // 10帧循环
    } else if (!isOnGround()) {
        // 在空中：固定显示 image25 (索引24)
        return 24; // 不管是否空翻都显示第25张图片
    } else {
        // 地面滑行：image1-13 (索引0-12)
        return m_currentFrame % 13; // 13帧循环
    }
}

// 修改updateAnimation方法
void Player::updateAnimation() {
    if (m_animationLoaded && !m_animationFrames.isEmpty()) {
        // 简单递增帧计数器
        m_currentFrame = (m_currentFrame + 1) % 100; // 使用较大的循环避免溢出

        // 根据状态计算实际要显示的帧
        int actualFrame = getCurrentAnimationRange();

        // 确保帧索引在有效范围内
        if (actualFrame >= 0 && actualFrame < m_animationFrames.size()) {
            m_currentFrame = actualFrame;
            // 触发重绘
            update();
        }
    }
}

void Player::keyPressEvent(QKeyEvent *event) {
    if (event->isAutoRepeat())
        return;

    switch (event->key()) {        case Qt::Key_Left:
            keyLeft = true;
            DEBUG_LOG("Left key pressed");
            break;        case Qt::Key_Right:
            keyRight = true;
            DEBUG_LOG("Right key pressed");
            break;        case Qt::Key_Space:
        case Qt::Key_Up:
            keySpace = true;
            DEBUG_LOG("Space key pressed");
        // 处理跳跃
            jump();
            break;
    }
}

void Player::keyReleaseEvent(QKeyEvent *event) {
    if (event->isAutoRepeat())
        return;

    switch (event->key()) {        case Qt::Key_Left:
            keyLeft = false;
            DEBUG_LOG("Left key released");
            break;        case Qt::Key_Right:
            keyRight = false;
            DEBUG_LOG("Right key released");
            break;
        case Qt::Key_Space:
        case Qt::Key_Up:
            keySpace = false;
            DEBUG_LOG("Space key released");
            break;
        case Qt::Key_T://显示调试信息
            DEBUG_LOG(QString("Rotation: %1").arg(QString::number(rotation)));
    }
}

void Player::jump() {
    if (is_fallen) {
        int rem = m_fallRecoveryTimer.remainingTime();
        int newRem = qMax(rem - 200, 0);
        m_fallRecoveryTimer.start(newRem);
        DEBUG_LOG(QString("Reducing fall recovery time by 200ms, new remaining: %1").arg(QString::number(newRem)));
        return;
    }
    if (!isOnGround()) {
        DEBUG_LOG("Jump is not available");
        return;
    }
    QPointF vel = velocity();
    vel.setY(m_jumpForce);
    setVelocity(vel);
    setOnGround(false);
    DEBUG_LOG("Jump");
}

void Player::setOnGround(bool onGround)
{
    if (m_onGround == onGround) return; // 状态未改变
    m_onGround = onGround;
    DEBUG_LOG(QString("Player on ground state changed to %1").arg(QString::number(onGround)));
}

// 当玩家离地（跳跃或从坡上飞出）时调用
void Player::notifyTakeoff() {
    m_takeoffRotation = rotation;
    m_cumulativeRotation = 0.0;     // 重置累计旋转角度
    m_lastFrameRotation = rotation; // 记录起始角度作为上一帧角度
    DEBUG_LOG(QString("Takeoff with angle: %1").arg(QString::number(m_takeoffRotation)));
}

// 检查落地角度并判断是否摔倒
void Player::checkLanding(qreal terrainAngle) {
    // 使用累计旋转角度而不是简单的角度差
    m_flipRotation = qAbs(m_cumulativeRotation);
    DEBUG_LOG(QString("Landing! Total flip rotation: %1 (%2 flips)")
      .arg(QString::number(m_flipRotation)).arg(QString::number(m_flipRotation/360.0)));

    // 检查是否需要摔倒
    if (!is_fallen) { // 确保不重复判断
        // 计算与地面的角度偏差
        qreal playerAngle = normalizeAngle(rotation);
        qreal normalizedTerrainAngle = normalizeAngle(terrainAngle);
        qreal angleDeviation = qAbs(playerAngle - normalizedTerrainAngle);

        // 处理超过180度的偏差
        if (angleDeviation > 180.0) {
            angleDeviation = 360.0 - angleDeviation;
        }
        DEBUG_LOG(QString("Landing angle check - Player: %1 Terrain: %2 Deviation: %3")
      .arg(QString::number(playerAngle)).arg(QString::number(normalizedTerrainAngle)).arg(QString::number(angleDeviation)));

        // 如果偏差过大且无法抵抗，则摔倒
        if (angleDeviation > MAX_LANDING_ANGLE_DEVIATION && !canResistFall(angleDeviation)) {
            fall();
        }
    }
}

void Player::checkHitRock(RockEntity* rock) {
    if (!rock) return;
    // 玩家摔倒
    fall();
    // 从场景移除石头、从物理系统注销和删除石头对象的操作
    // 现在由 GameScene::handlePhysicsObjectCollision 处理，以支持延迟删除。
}

// 覆盖getTargetVelocityX来禁止摔倒时移动
qreal Player::getTargetVelocityX() const {
    if (is_fallen) {
        return 0; // 摔倒时不移动
    }

    // 原有代码
    qreal targetVelocity = 0;
    if (keyLeft) {
        targetVelocity -= m_moveSpeed;
    }
    if (keyRight) {
        targetVelocity += m_moveSpeed;
    }
    return targetVelocity;
}

// 修改updateRotate处理摔倒姿势
void Player::updateRotate(TerrainGenerator* GTerrainGenerator) {
    if (is_fallen) {
        // 摔倒姿势 - 侧躺
        setRotation(90); // 简单的90度侧躺姿势
        return;
    }

    qreal oldRotation = rotation;

    // 当在空中且按下Space键时，旋转,若没按下，则缓慢回到地形角度
    if (isOnGround() || !keySpace) {
        // 计算当前角度归一化值（角度/360）
        qreal normalizedAngle = rotation / 360.0;

        // 目标角度固定为60度
        qreal targetAngle = 60.0;

        // 以0.5度的角速度平滑过渡到目标角度
        qreal rotateSpeed = 0.5;
        qreal angle = rotation;

        if (normalizedAngle < targetAngle) {
            angle += rotateSpeed;
            if (angle > targetAngle) {
                angle = targetAngle;
            }
        } else if (normalizedAngle > targetAngle) {
            angle -= rotateSpeed;
            if (angle < targetAngle) {
                angle = targetAngle;
            }
        }

        setRotation(angle);
    } else {
        //如果在空中且按下Space键，则以成员变量的角速度顺时针旋转
        qreal angle = rotation;
        angle -= rotateSpeed; // 改为减法，实现顺时针旋转
        setRotation(angle);
    }

    // 在空中时累加旋转角度变化
    if (!isOnGround()) {
        // 计算本帧旋转了多少度（处理角度溢出）
        qreal rotationDelta = rotation - m_lastFrameRotation;

        // 处理角度溢出（例如从359度到1度的变化应该是+2而不是-358）
        if (rotationDelta > 180) {
            rotationDelta -= 360;
        } else if (rotationDelta < -180) {
            rotationDelta += 360;
        }

        // 累加到总旋转角度
        m_cumulativeRotation += rotationDelta;

        // 更新上一帧角度
        m_lastFrameRotation = rotation;
    }
}

// 摔倒相关方法
void Player::fall() {
    if (is_fallen) return;

    is_fallen = true;
    DEBUG_LOG(QString("Player has fallen! Flip rotation was: %1").arg(QString::number(m_flipRotation)));

    // 启动恢复计时器
    m_fallRecoveryTimer.start(3000); // 3秒后恢复
}

void Player::recoverFromFall() {
    if (!is_fallen) return;

    is_fallen = false;
    DEBUG_LOG("Player recovered from fall.");
}

void Player::onFallRecoveryTimeout() {
    recoverFromFall();
}

bool Player::canResistFall(qreal angleDeviation) const {
    // 未来可扩展为返回true的条件
    return false;
}

qreal Player::getFlipRotation() const {
    return m_flipRotation;
}

bool Player::isFallen() const {
    return is_fallen;
}

void Player::setImageScaleFactor(qreal factor) {
    // 限制缩放因子在合理范围内，防止图像过大或过小
    m_imageScaleFactor = qBound(0.5, factor, 3.0);
    update(); // 触发重绘
}

qreal Player::imageScaleFactor() const {
    return m_imageScaleFactor;
}

void Player::playerUpdate(TerrainGenerator* GTerrainGenerator) {
    // 处理旋转
    updateRotate(GTerrainGenerator);

}


void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    // 如果动画已加载且有帧数据，绘制当前动画帧
    if (m_animationLoaded && !m_animationFrames.isEmpty() &&
        m_currentFrame >= 0 && m_currentFrame < m_animationFrames.size()) {        QRectF r = rect();
        const QPixmap& currentPixmap = m_animationFrames[m_currentFrame];        // 保存当前绘图设置
        painter->save();

        // 设置高质量渲染选项
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter->setRenderHint(QPainter::Antialiasing, true);

        // 使用自定义缩放因子计算绘制区域
        QSizeF size = r.size() * m_imageScaleFactor;
        QRectF targetRect(
            r.x() + (r.width() - size.width()) / 2,
            r.y() + (r.height() - size.height()) / 2,
            size.width(),
            size.height() + 2 // 增加5像素高度以适应底部
        );

        // 在绘制时进行缩放
        painter->drawPixmap(targetRect, currentPixmap, currentPixmap.rect());

        // 恢复绘图设置
        painter->restore();
        // 已移除
        // 底部绿色边框作为调试标识
        // QPen greenPen(Qt::green, 2);
        // painter->setPen(greenPen);
        // painter->drawLine(r.bottomLeft(), r.bottomRight());

    } else {
        // 如果动画未加载，使用原始的红色方块绘制
        QGraphicsRectItem::paint(painter, option, widget);

        // 绘制底部绿色边
        QRectF r = rect();
        QPen greenPen(Qt::green, 4);
        painter->setPen(greenPen);
        painter->drawLine(r.bottomLeft(), r.bottomRight());
    }
}