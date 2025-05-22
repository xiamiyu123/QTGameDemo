#include "player.h"
#include <QBrush>
#include <QPen>
#include <QDebug>
#include <qpainter.h>
#include <QGraphicsScene>
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
    : BasePhysicsEntity(40, 40, parent),
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
      m_drawOffsetX(-20),    // 调整绘制偏移，使角色正确显示
      m_drawOffsetY(-25),    // 垂直绘制偏移，确保角色站在雪地上
      m_lastFrameRotation(0.0) {

    setZValue(-2);

    // 设置玩家外观

    // 设置实体类型
    setEntityType(EntityType::Player);

    // 加载所有玩家图像
    m_playerImages.reserve(44);
    for (int i = 0; i <= 43; i++) {
        QString path = QString(":/resource/images/player/image%1.png").arg(QString::number(i));
        m_playerImages.append(QPixmap(path));
    }

    // 设置当前图像索引
    m_currentImageIndex = 0;

    // 设置动画状态
    m_animState = Standing;

    // 设置动画计时器
    m_animTimer = 0;

    // 允许接收键盘焦点
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFocus();

    // 初始化摔倒恢复计时器
    connect(&m_fallRecoveryTimer, &QTimer::timeout, this, &Player::onFallRecoveryTimeout);
    m_fallRecoveryTimer.setSingleShot(true);
    // 默认朝向
    m_facingRight = true;
}

Player::~Player() {
    // 父类析构函数会处理注销和组件删除
}

void Player::keyPressEvent(QKeyEvent *event) {
    if (event->isAutoRepeat())
        return;

    switch (event->key()) {
        case Qt::Key_Left:
            keyLeft = true;
            m_facingRight = false;
            qDebug() << "Left key pressed";
            break;
        case Qt::Key_Right:
            keyRight = true;
            m_facingRight = true;
            qDebug() << "Right key pressed";
            break;
        case Qt::Key_Space:
        case Qt::Key_Up:
            keySpace = true;
            qDebug() << "Space key pressed";
        // 处理跳跃
            jump();
            break;
    }
}

void Player::keyReleaseEvent(QKeyEvent *event) {
    if (event->isAutoRepeat())
        return;

    switch (event->key()) {
        case Qt::Key_Left:
            keyLeft = false;
            qDebug() << "Left key released";
            break;
        case Qt::Key_Right:
            keyRight = false;
            qDebug() << "Right key released";
            break;
        case Qt::Key_Space:
        case Qt::Key_Up:
            keySpace = false;
            qDebug() << "Space key released";
            break;
        case Qt::Key_T://显示调试信息
            qDebug() << "Rotation:" << rotation;
    }
}

void Player::jump() {
    if (is_fallen) {
        int rem = m_fallRecoveryTimer.remainingTime();
        int newRem = qMax(rem - 200, 0);
        m_fallRecoveryTimer.start(newRem);
        qDebug() << "Reducing fall recovery time by 200ms, new remaining:" << newRem;
        return;
    }
    if (!isOnGround()) {
        qDebug() << "Jump is not available";
        return;
    }
    QPointF vel = velocity();
    vel.setY(m_jumpForce);
    setVelocity(vel);
    setOnGround(false);
    qDebug() << "Jump";
}

void Player::setOnGround(bool onGround)
{
    if (m_onGround == onGround) return; // 状态未改变
    m_onGround = onGround;
    qDebug ()<<"Player on ground state changed to " << onGround;
}

// 当玩家离地（跳跃或从坡上飞出）时调用
void Player::notifyTakeoff() {
    m_takeoffRotation = rotation;
    m_cumulativeRotation = 0.0;     // 重置累计旋转角度
    m_lastFrameRotation = rotation; // 记录起始角度作为上一帧角度
    qDebug() << "Takeoff with angle:" << m_takeoffRotation;
    // 设置动画状态
    m_animState = Flipping;
    m_animTimer = 0;
}

// 检查落地角度并判断是否摔倒
void Player::checkLanding(qreal terrainAngle) {
    // 使用累计旋转角度而不是简单的角度差
    m_flipRotation = qAbs(m_cumulativeRotation);
    
    qDebug() << "Landing! Total flip rotation:" << m_flipRotation 
             << "(" << m_flipRotation/360.0 << " flips)";
    
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
        
        qDebug() << "Landing angle check - Player:" << playerAngle
                 << "Terrain:" << normalizedTerrainAngle
                 << "Deviation:" << angleDeviation;
        
        // 如果偏差过大且无法抵抗，则摔倒
        if (angleDeviation > MAX_LANDING_ANGLE_DEVIATION && !canResistFall(angleDeviation)) {
            fall();
        }
    }
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
    qDebug() << "Player has fallen! Flip rotation was:" << m_flipRotation;

    // 更新动画状态
    m_animState = Falling;
    m_animTimer = 0;

    // 启动恢复计时器
    m_fallRecoveryTimer.setSingleShot(true);

    // 启动恢复计时器
    m_fallRecoveryTimer.start(3000); // 3秒后恢复
}

void Player::recoverFromFall() {
    if (!is_fallen) return;
    
    is_fallen = false;
    qDebug() << "Player recovered from fall.";
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

void Player::playerUpdate(TerrainGenerator* GTerrainGenerator) {
    // 处理旋转
    updateAnimation();

    updateRotate(GTerrainGenerator);

}


void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // 保存当前画家状态
    painter->save();

    // 绘制实际图像
    QPixmap currentImage = m_playerImages[m_currentImageIndex];

    // 创建变换以居中绘制图像
    QTransform transform;

    // 如果正在空翻，使用独立的旋转
    if (m_animState == Flipping) {
        transform.translate(rect().width()/2, rect().height()/2);
        transform.rotate(m_cumulativeRotation);
        transform.translate(-rect().width()/2, -rect().height()/2);
    }

    // 根据朝向翻转图像
    if (!m_facingRight) {
        transform.translate(rect().width(), 0);
        transform.scale(-1, 1);
    }

    painter->setTransform(transform, true);

    // 绘制图像，居中于碰撞盒
    painter->drawPixmap(m_drawOffsetX, m_drawOffsetY, currentImage);

    // 恢复画家状态
    painter->restore();




}

void Player::updateAnimation()
{
    // Increase animation timer (assume 60fps)
    m_animTimer += 0.016f;

    switch (m_animState) {
    case Standing:
        // Standing animation remains unchanged: images 0-3
            m_currentImageIndex = static_cast<int>(m_animTimer * 8) % 4;
        break;
    case Running:
        // Running animation: images 8-12 (5 images cycling)
            m_currentImageIndex = 8 + (static_cast<int>(m_animTimer * 12) % 5);
        break;
    case Jumping:
        // Jumping animation remains unchanged: images 20-25 (non-looping)
            m_currentImageIndex = qMin(20 + static_cast<int>(m_animTimer * 10), 25);
        break;
    case Flipping:
        // Flipping animation: images 13-26 (14 images cycling)
            m_currentImageIndex = 13 + (static_cast<int>(m_animTimer * 12) % 14);
        break;
    case Falling:
        // Falling state: if rotation is near 90º (head down), use image 38;
            // otherwise use flipping landing buffer: images 1-7 (7 images cycling)
                if (qAbs(rotation - 90) < 10) {
                    m_currentImageIndex = 38;
                } else {
                    m_currentImageIndex = 1 + (static_cast<int>(m_animTimer * 8) % 7);
                }
        break;
    }
}