#include "player.h"
#include <QBrush>
#include <QPen>
#include "debuglogger.h"
#include <qpainter.h>
#include "npcentity.h"
#include <vector>

#include "gamescene.h"
#include "terraingenerator.h"

// 定义静态常量
const qreal Player::MAX_LANDING_ANGLE_DEVIATION = 45.0; // 45度最大偏差

// 企鹅形态的属性加成配置
const Player::NPCFormModifiers Player::PENGUIN_MODIFIERS = {
    1.2,    // 移动速度 x1.2
    1.1,    // 跳跃速度 x1.1
    1.2,    // 空翻速度 x1.2
    0,      // 库存容量加成为0（保持基础值）
    true,   // 可以空翻
    0       // 企鹅携带容量为0
};

// 雪怪形态1的属性加成配置（初始骑乘）
const Player::NPCFormModifiers Player::YETI_FORM1_MODIFIERS = {
    1.5,    // 移动速度 x1.5 (+50%)
    1.2,    // 跳跃速度 x1.2
    1.0,    // 空翻速度 x1.0（不变，但被禁用）
    0,      // 库存容量加成为0
    false,  // 不可以空翻
    0       // 不可以携带企鹅（雪怪形态1没有额外企鹅存储位）
};

// 雪怪形态2的属性加成配置（被撞击后）
const Player::NPCFormModifiers Player::YETI_FORM2_MODIFIERS = {
    1.3,    // 移动速度 x1.3 (+30%)
    1.1,    // 跳跃速度 x1.1
    1.0,    // 空翻速度 x1.0（正常）
    0,      // 库存容量加成为0
    true,   // 可以空翻
    1       // 可以携带1只企鹅
};

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
      m_imageScaleFactor(1),
      m_flipBoostTimer(nullptr),
      m_isFlipBoosting(false),
      m_terrainGenerator(nullptr), // 初始化地形生成器指针
      m_currentForm(NPCForm::Normal), // 初始化为普通形态
      m_baseMoveSpeed(500),
      m_baseJumpForce(-300),
      m_baseFlipSpeed(3),
      m_baseInventoryCapacity(1),
      m_currentMoveSpeed(500),
      m_currentJumpForce(-300),
      m_currentFlipSpeed(3),
      m_currentInventoryCapacity(1),
      m_isRidingYeti(false), // 初始化为未骑乘雪怪
      m_yetiForm(NPCForm::Normal) // 初始化雪怪形态为普通
{
    setZValue(-2);

    initialMoveSpeed = m_moveSpeed; // 保存初始移动速度

    // 设置玩家外观
    setBrush(QBrush(Qt::red));
    setPen(QPen(Qt::black, 2));

    // 设置实体类型
    setEntityType(EntityType::Player);    // 允许接收键盘焦点
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFocus();

    // 初始化摔倒恢复计时器
    connect(&m_fallRecoveryTimer, &QTimer::timeout, this, &Player::onFallRecoveryTimeout);
    m_fallRecoveryTimer.setSingleShot(true);
      // 初始化摔倒恢复进度更新定时器
    connect(&m_fallRecoveryProgressTimer, &QTimer::timeout, this, &Player::updateFallRecoveryProgress);
    m_fallRecoveryProgressTimer.setSingleShot(false); // 重复触发

    // 初始化NPC拾取冷却系统
    m_npcPickupCooldownActive = false;
    connect(&m_npcPickupCooldownTimer, &QTimer::timeout, this, &Player::onNPCPickupCooldownTimeout);
    m_npcPickupCooldownTimer.setSingleShot(true);

    // 初始化NPC冷却进度更新定时器
    connect(&m_npcCooldownProgressTimer, &QTimer::timeout, this, &Player::updateNPCCooldownProgress);
    m_npcCooldownProgressTimer.setSingleShot(false); // 重复触发

    // 连接NPC状态更新信号和槽
    connect(this, &Player::updatePlayerNPC, this, &Player::onUpdate);

    loadAnimationFrames();

    // 设置动画定时器
    connect(&m_animationTimer, &QTimer::timeout, this, &Player::updateAnimation);
    m_animationTimer.start(100); // 每100毫秒更新一帧，约10FPS    // 初始化空翻加速计时器
    m_flipBoostTimer = new QTimer(this);
    m_flipBoostTimer->setSingleShot(true);
    connect(m_flipBoostTimer, &QTimer::timeout, this, &Player::onFlipBoostTimerTimeout);

    // 初始化空翻加速进度更新计时器
    m_flipBoostProgressTimer = new QTimer(this);
    m_flipBoostProgressTimer->setSingleShot(false); // 重复触发
    connect(m_flipBoostProgressTimer, &QTimer::timeout, this, &Player::updateFlipBoostProgress);
}

Player::~Player() {
    // 释放空翻加速计时器
    if (m_flipBoostTimer) {
        delete m_flipBoostTimer;
        m_flipBoostTimer = nullptr;
    }

    // 释放空翻加速进度更新计时器
    if (m_flipBoostProgressTimer) {
        delete m_flipBoostProgressTimer;
        m_flipBoostProgressTimer = nullptr;
    }

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
            break;        case Qt::Key_X:
            // 智能丢弃NPC：优先丢弃携带的企鹅，然后丢弃主库存中的低优先级NPC
            if (m_isRidingYeti && !m_penguinCarryInventory.empty()) {
                // 骑乘雪怪时，优先丢弃携带的企鹅
                dropCarriedPenguin();
                DEBUG_LOG("Dropped carried penguin while riding Yeti");
            } else if (hasNPCInInventory() && m_terrainGenerator) {
                // 丢弃主库存中的NPC
                dropNPC(m_terrainGenerator);
            } else if (!hasNPCInInventory() && m_penguinCarryInventory.empty()) {
                DEBUG_LOG("No NPCs in inventory to drop");
            } else {
                DEBUG_LOG("TerrainGenerator not available for NPC dropping");
            }
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
        } else {
            // 检查空翻条件
            if (m_flipRotation >= 600.0) {
                // 多圈空翻成功！给予更高奖励
                emit backFlipSuccess(500, "超级空翻！");
                DEBUG_LOG("多圈空翻成功! 奖励 +500 分");
            } else if (m_flipRotation >= 200.0) {
                // 普通空翻成功
                emit backFlipSuccess(200, "后空翻！");
                DEBUG_LOG("空翻成功! 奖励 +200 分");
            }

            // 重置累计旋转角度，避免再次触发
            m_cumulativeRotation = 0.0;
        }
    }
}

void Player::checkHitRock(RockEntity* rock) {
    if (!rock) return;

    // 如果玩家正在骑乘雪怪形态1，先转换为形态2
    if (m_isRidingYeti && m_yetiForm == NPCForm::YetiForm1) {
        transformYetiForm();
        DEBUG_LOG("Yeti transformed from Form1 to Form2 due to rock collision");
        return; // 雪怪形态1遇到碰撞时只转换形态，不摔倒
    }

    // 其他情况下正常处理摔倒
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

    qreal oldRotation = rotation;    // 当在空中且按下Space键时，旋转,若没按下，则缓慢回到地形角度
    if (isOnGround() || !keySpace) {
        // 计算当前角度归一化值（角度/360）
        qreal normalizedAngle = rotation / 360.0;

        // 目标角度：骑乘雪怪时为0度，其他情况为60度
        qreal targetAngle = m_isRidingYeti ? 0.0 : 60.0;

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
        // 如果在空中且按下Space键，检查是否可以空翻
        if (canFlip()) {
            // 以成员变量的角速度顺时针旋转
            qreal angle = rotation;
            angle -= rotateSpeed; // 改为减法，实现顺时针旋转
            setRotation(angle);
        } else {
            // 雪怪形态1等不能空翻的状态，保持当前角度
            // DEBUG_LOG("Flip is disabled in current form");
        }
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

    // 检查是否可以通过消耗NPC来抵抗摔倒
    if (canResistFall(0.0)) { // 传入0作为角度偏差参数
        DEBUG_LOG("Player successfully resisted fall");
        return; // 成功抵抗，不摔倒
    }

    is_fallen = true;
    DEBUG_LOG(QString("Player has fallen! Flip rotation was: %1").arg(QString::number(m_flipRotation)));

    // 启动恢复计时器
    m_fallRecoveryTimer.start(3000); // 3秒后恢复

    // // 发射摔倒信号（测试用）
    // emit playerFallen(100, "FALL！");

    // 启动摔倒恢复进度更新定时器
    m_fallRecoveryProgressTimer.start(50); // 每50毫秒更新一次进度

    // 发出摔倒恢复开始信号
    emit fallRecoveryChanged(true, 0.0);

    // 触发重置倍率信号
    emit resetAwardMultipliers();
}

void Player::recoverFromFall() {
    if (!is_fallen) return;

    is_fallen = false;

    // 重置累计旋转角度，避免摔倒后站起来时仍然保留旋转角度
    m_cumulativeRotation = 0.0;
    m_flipRotation = 0.0;
    m_lastFrameRotation = rotation;

    DEBUG_LOG("Player recovered from fall.");
}

void Player::onFallRecoveryTimeout() {
    // 停止摔倒恢复进度更新定时器
    m_fallRecoveryProgressTimer.stop();

    // 发出摔倒恢复结束信号
    emit fallRecoveryChanged(false, 0.0);

    recoverFromFall();
}

bool Player::canResistFall(qreal angleDeviation) {
    Q_UNUSED(angleDeviation); // 暂时不使用角度偏差参数

    // 优先检查是否有携带的企鹅可以用来抵抗摔倒
    if (!m_penguinCarryInventory.empty()) {
        if (consumePenguinForDamageResistance()) {
            DEBUG_LOG("Player resisted fall by consuming a carried penguin");
            return true;
        }
    }

    // 检查是否有主库存NPC可以用来抵抗摔倒
    if (hasNPCInInventory()) {
        // 消耗最低优先级的NPC来抵抗摔倒
        if (consumeNPCForDamageResistance()) {
            DEBUG_LOG("Player resisted fall by consuming an NPC from main inventory");
            return true;
        }
    }

    //检查是否处于空翻后的加速状态
    if (m_isFlipBoosting) {
        // 如果正在空翻加速中，允许抵抗摔倒
        DEBUG_LOG("Player resisted fall by being in flip boosting state");
        return true;
    }

    // 未来可扩展为其他抵抗条件
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
    // 更新地形生成器引用
    m_terrainGenerator = GTerrainGenerator;

    // 处理旋转
    updateRotate(GTerrainGenerator);

}


void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)    // 如果动画已加载且有帧数据，绘制当前动画帧
    if (m_animationLoaded && !m_animationFrames.isEmpty() &&
        m_currentFrame >= 0 && m_currentFrame < m_animationFrames.size()) {

        QRectF r = rect();
        const QPixmap& currentPixmap = m_animationFrames[m_currentFrame];

        // 保存当前绘图设置
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

void Player::setMoveSpeed(qreal speed) {
    m_moveSpeed = speed;
    DEBUG_LOG(QString("玩家速度设置为: %1").arg(QString::number(m_moveSpeed)));
}

qreal Player::moveSpeed() const {
    return m_moveSpeed;
}

bool Player::isFlipBoosting() const {
    return m_isFlipBoosting;
}

void Player::startFlipBoost()
{
    // 激活空翻加速状态
    m_isFlipBoosting = true;

    // 启动计时器，2秒后关闭加速
    m_flipBoostTimer->start(1000);
    m_flipBoostTimer->start(FLIP_BOOST_TIME_MS); // 2000毫秒 = 2秒

    // 启动进度更新计时器
    m_flipBoostProgressTimer->start(FLIP_BOOST_PROGRESS_UPDATE_MS);

    // 发出空翻加速开始信号，初始进度为0
    emit flipBoostChanged(true, 0.0);

    DEBUG_LOG("空翻加速激活，持续2秒");
}

void Player::onFlipBoostTimerTimeout()
{
    m_isFlipBoosting = false;

    // 恢复初速度
    setMoveSpeed(getInitialMoveSpeed());

    // 停止进度更新计时器
    m_flipBoostProgressTimer->stop();

    // 发出空翻加速结束信号
    emit flipBoostChanged(false, 0.0);

    DEBUG_LOG("空翻加速效果结束");
}


// === NPC库存系统实现 ===

bool Player::pickupNPC(NPCEntity* npc) {
    if (!npc) {
        DEBUG_LOG("Player::pickupNPC - NPC is null");
        return false;
    }
      // 检查拾取冷却状态
    if (!canPickupNPC()) {
        DEBUG_LOG("Player::pickupNPC - Pickup on cooldown");
        return false;
    }

    // 获取NPC的ID
    int npcId = npc->class_id();    // 特殊处理企鹅：如果正在骑乘雪怪，直接添加到携带库存，不占用主库存
    if (npcId == 1 && m_isRidingYeti) { // PenguinNPC::ID
        NPCFormModifiers currentModifiers = getFormModifiers(m_currentForm);
        if (m_penguinCarryInventory.size() < currentModifiers.penguinCarryCapacity) {
            addPenguinToCarry(npcId);
            DEBUG_LOG("Added penguin to carry inventory while riding Yeti");

            // 标记NPC为待删除
            npc->markForDestroy();

            // 触发奖励信号
            emit npcCaptureSuccess(200, "出租车！", npcId);
            DEBUG_LOG("Emitted npcCaptureSuccess signal for carried penguin: +150 points");

            // 触发玩家NPC状态更新信号
            emit updatePlayerNPC();
            return true;
        } else {
            DEBUG_LOG("Cannot carry more penguins - carry capacity reached");
            return false; // 携带位满时返回false，不删除企鹅
        }
    }

    // 检查库存是否已满
    if (m_npcInventory.size() >= getCurrentInventoryCapacity()) {
        DEBUG_LOG("Player::pickupNPC - Inventory is full");

        // 检查新NPC的优先级是否高于库存中的最低优先级NPC
        std::priority_queue<int> tempQueue = m_npcInventory;
        int lowestPriorityId = tempQueue.top();

        // 找到最小ID（最低优先级）
        while (!tempQueue.empty()) {
            int id = tempQueue.top();
            if (id < lowestPriorityId) {
                lowestPriorityId = id;
            }
            tempQueue.pop();
        }        // 如果新NPC优先级不高于现有最低优先级，拒绝拾取
        // 注意：ID值越大优先级越高，所以使用小于等于比较
        if (npcId <= lowestPriorityId) {
            DEBUG_LOG(QString("Rejecting NPC pickup - new NPC priority (%1) not higher than lowest existing (%2)")
                      .arg(npcId).arg(lowestPriorityId));
            return false;
        }

        // 新NPC优先级更高，扔出最低优先级的NPC
        DEBUG_LOG("Player::pickupNPC - Dropping lower priority NPC to make space");
        dropNPC(m_terrainGenerator);
    }

    // 添加NPC到库存
    m_npcInventory.push(npcId);

    // 根据拾取的NPC类型应用对应的形态和发出奖励信号
    if (npcId == 1) { // PenguinNPC::ID
        applyNPCForm(NPCForm::Penguin);
        emit npcCaptureSuccess(200, "企鹅滑雪！", npcId);
        DEBUG_LOG("Player transformed into Penguin form and emitted npcCaptureSuccess signal: +200 points");
    } else if (npcId == 2) { // YetiNPC::ID
        m_isRidingYeti = true;
        m_yetiForm = NPCForm::YetiForm1; // 初始为形态1
        applyNPCForm(NPCForm::YetiForm1);
        emit npcCaptureSuccess(200, "走你！", npcId);
        DEBUG_LOG("Player mounted Yeti in Form1 and emitted npcCaptureSuccess signal: +400 points");
    }

    DEBUG_LOG(QString("Player picked up NPC with ID: %1, inventory size: %2")
              .arg(npcId).arg(m_npcInventory.size()));

    // 标记NPC为待删除
    npc->markForDestroy();
      // 触发玩家NPC状态更新信号
    emit updatePlayerNPC();

    return true; // 成功拾取
}

void Player::dropNPC() {
    // 简单版本，只记录日志
    if (m_npcInventory.empty()) {
        DEBUG_LOG("Player::dropNPC - No NPCs in inventory");
        return;
    }

    DEBUG_LOG("Player::dropNPC - Please use dropNPC(TerrainGenerator*) to actually spawn NPCs");
}

void Player::dropNPC(TerrainGenerator* terrainGenerator) {
    if (m_npcInventory.empty()) {
        DEBUG_LOG("Player::dropNPC - No NPCs in inventory");
        return;
    }

    if (!terrainGenerator) {
        DEBUG_LOG("Player::dropNPC - TerrainGenerator is null");
        return;
    }

    // 获取优先级最低的NPC（堆顶是最高优先级，我们需要最低的）
    // 由于priority_queue是最大堆，我们需要遍历找到最小值
    std::priority_queue<int> tempQueue = m_npcInventory;
    int lowestPriorityId = tempQueue.top();

    // 找到最小ID（最低优先级）
    std::vector<int> allIds;
    while (!tempQueue.empty()) {
        int id = tempQueue.top();
        allIds.push_back(id);
        if (id < lowestPriorityId) {
            lowestPriorityId = id;
        }
        tempQueue.pop();
    }

    // 重建队列，只移除一个最低优先级的NPC
    m_npcInventory = std::priority_queue<int>();
    bool removedOne = false;
    for (int id : allIds) {
        if (id == lowestPriorityId && !removedOne) {
            // 跳过第一个遇到的最低优先级NPC（将其丢弃）
            removedOne = true;
        } else {
            // 保留其他所有NPC
            m_npcInventory.push(id);
        }
    }

    DEBUG_LOG(QString("Player dropped NPC with ID: %1, remaining inventory size: %2")
              .arg(lowestPriorityId).arg(m_npcInventory.size()));
      // 在玩家位置生成对应ID的NPC
    QPointF spawnPosition = pos();

    // 计算生成位置的地面高度
    qreal terrainHeight = terrainGenerator->getTerrainHeight(spawnPosition.x());


    spawnPosition.setY(terrainHeight);

      // 根据ID创建对应的NPC
    std::unique_ptr<NPCEntity> newNPC = nullptr;
    if (lowestPriorityId == 1) { // PenguinNPC::ID
        newNPC = NPCFactory::createPenguinNPC(spawnPosition);
    } else if (lowestPriorityId == 2) { // YetiNPC::ID
        newNPC = NPCFactory::createYetiNPC(spawnPosition);
        // 如果丢弃雪怪，玩家停止骑乘状态
        m_isRidingYeti = false;
        m_yetiForm = NPCForm::Normal;
    }
    // 可以在这里添加其他NPC类型的创建逻辑

    if (newNPC) {
        // 将NPC添加到场景和地形生成器
        if (scene()) {
            scene()->addItem(newNPC.get());
        }

        // 注册到物理系统
        PhysicsSystem::instance().registerObject(newNPC.get());

        // 添加到地形生成器的NPC列表
        terrainGenerator->m_npcs.append(newNPC.release());        DEBUG_LOG(QString("Successfully spawned NPC with ID %1 at position (%2, %3)")
                  .arg(lowestPriorityId).arg(spawnPosition.x()).arg(spawnPosition.y()));

        // 启动拾取冷却 - 玩家失去NPC后1秒内不能再拾起NPC
        startNPCPickupCooldown();

        // 检查是否需要更新玩家形态
        updatePlayerFormBasedOnInventory();

        // 触发玩家NPC状态更新信号
        emit updatePlayerNPC();
    } else {
        DEBUG_LOG(QString("Failed to create NPC with ID %1").arg(lowestPriorityId));
    }
}

bool Player::hasNPCInInventory() const {
    return !m_npcInventory.empty();
}

int Player::getInventorySize() const {
    return static_cast<int>(m_npcInventory.size());
}

// === NPC拾取冷却系统实现 ===

bool Player::canPickupNPC() const {
    return !m_npcPickupCooldownActive;
}

void Player::startNPCPickupCooldown() {
    if (!m_npcPickupCooldownActive) {
        m_npcPickupCooldownActive = true;
        m_npcPickupCooldownTimer.start(NPC_PICKUP_COOLDOWN_MS);

        // 启动进度更新定时器
        m_npcCooldownProgressTimer.start(NPC_COOLDOWN_PROGRESS_UPDATE_MS);

        // 发出冷却开始信号
        emit npcPickupCooldownChanged(true, 0.0);

        DEBUG_LOG(QString("NPC pickup cooldown started - 3 seconds"));
    }
}

void Player::onNPCPickupCooldownTimeout() {
    m_npcPickupCooldownActive = false;

    // 停止进度更新定时器
    m_npcCooldownProgressTimer.stop();

    // 发出冷却结束信号
    emit npcPickupCooldownChanged(false, 0.0);

    DEBUG_LOG("NPC pickup cooldown ended - can pickup NPCs again");
}

void Player::updateNPCCooldownProgress() {
    if (!m_npcPickupCooldownActive) {
        return; // 如果没有冷却，不需要更新进度
    }

    // 计算剩余时间
    int remainingTime = m_npcPickupCooldownTimer.remainingTime();

    // 计算进度（0.0 表示刚开始，1.0 表示即将结束）
    qreal progress = 1.0 - (static_cast<qreal>(remainingTime) / static_cast<qreal>(NPC_PICKUP_COOLDOWN_MS));

    // 确保进度在有效范围内
    progress = qBound(0.0, progress, 1.0);

    // 发出进度更新信号
    emit npcPickupCooldownChanged(true, progress);
}

void Player::onUpdate() {
    // TODO: 在这里实现玩家状态更新逻辑
    // 当NPC库存发生变化时，可以在这里处理相关的状态更新
    // 例如：更新UI显示、改变玩家属性、触发特殊效果等

    DEBUG_LOG(QString("Player NPC status updated - Current inventory size: %1")
              .arg(getInventorySize()));
}

// === NPC伤害抵抗系统实现 ===

bool Player::consumeNPCForDamageResistance() {
    if (m_npcInventory.empty()) {
        DEBUG_LOG("Player::consumeNPCForDamageResistance - No NPCs in inventory");
        return false;
    }

    if (!m_terrainGenerator) {
        DEBUG_LOG("Player::consumeNPCForDamageResistance - TerrainGenerator not available");
        return false;
    }

    // 复用dropNPC方法来丢弃最低优先级的NPC
    DEBUG_LOG("Player using NPC for damage resistance - dropping NPC");
    dropNPC(m_terrainGenerator);

    return true; // 成功消耗了NPC
}
  void Player::updateFallRecoveryProgress() {
    if (!is_fallen) {
        return; // 如果没有摔倒，不需要更新进度
    }

    // 计算剩余时间
    int remainingTime = m_fallRecoveryTimer.remainingTime();

    // 计算进度（0.0 表示刚开始，1.0 表示即将结束）
    const int FALL_RECOVERY_TIME_MS = 3000; // 摔倒恢复时间
    qreal progress = 1.0 - (static_cast<qreal>(remainingTime) / static_cast<qreal>(FALL_RECOVERY_TIME_MS));

    // 确保进度在有效范围内
    progress = qBound(0.0, progress, 1.0);

    // 发出进度更新信号
    emit fallRecoveryChanged(true, progress);
}

// === NPC形态系统实现 ===

Player::NPCForm Player::getCurrentForm() const {
    return m_currentForm;
}

void Player::applyNPCForm(NPCForm form) {
    if (m_currentForm == form) {
        return; // 形态未改变
    }

    m_currentForm = form;
    updatePlayerAttributes();

    DEBUG_LOG(QString("Player form changed to: %1").arg(static_cast<int>(form)));
}

void Player::updatePlayerAttributes() {
    if (m_currentForm == NPCForm::Normal) {
        // 恢复到基础属性
        m_currentMoveSpeed = m_baseMoveSpeed;
        m_currentJumpForce = m_baseJumpForce;
        m_currentFlipSpeed = m_baseFlipSpeed;
        m_currentInventoryCapacity = m_baseInventoryCapacity;
    } else {
        // 应用对应形态的属性加成
        NPCFormModifiers modifiers = getFormModifiers(m_currentForm);
        applyFormModifiers(modifiers);
    }

    // 更新实际使用的属性值
    m_moveSpeed = m_currentMoveSpeed;
    m_jumpForce = m_currentJumpForce;
    rotateSpeed = m_currentFlipSpeed;

    DEBUG_LOG(QString("Player attributes updated - Speed: %1, Jump: %2, Flip: %3, Inventory: %4")
              .arg(m_currentMoveSpeed)
              .arg(m_currentJumpForce)
              .arg(m_currentFlipSpeed)
              .arg(m_currentInventoryCapacity));
}

qreal Player::getCurrentMoveSpeed() const {
    return m_currentMoveSpeed;
}

qreal Player::getCurrentJumpForce() const {
    return m_currentJumpForce;
}

qreal Player::getCurrentFlipSpeed() const {
    return m_currentFlipSpeed;
}

int Player::getCurrentInventoryCapacity() const {
    return m_currentInventoryCapacity;
}

void Player::resetToNormalForm() {
    applyNPCForm(NPCForm::Normal);
}

Player::NPCFormModifiers Player::getFormModifiers(NPCForm form) const {
    switch (form) {
        case NPCForm::Penguin:
            return PENGUIN_MODIFIERS;
        case NPCForm::YetiForm1:
            return YETI_FORM1_MODIFIERS;
        case NPCForm::YetiForm2:
            return YETI_FORM2_MODIFIERS;        case NPCForm::Normal:
        default:
            return NPCFormModifiers(); // 默认无加成
    }
}

void Player::applyFormModifiers(const NPCFormModifiers& modifiers) {
    m_currentMoveSpeed = m_baseMoveSpeed * modifiers.moveSpeedMultiplier;
    m_currentJumpForce = m_baseJumpForce * modifiers.jumpForceMultiplier;
    m_currentFlipSpeed = m_baseFlipSpeed * modifiers.flipSpeedMultiplier;
    m_currentInventoryCapacity = m_baseInventoryCapacity + modifiers.inventoryCapacityBonus;
}

void Player::updatePlayerFormBasedOnInventory() {
    if (m_npcInventory.empty()) {
        // 库存为空，恢复到普通形态
        resetToNormalForm();
        DEBUG_LOG("Player inventory empty, reverting to normal form");
        return;
    }
      // 根据库存中的NPC确定应该使用的形态
    // 优先级：雪怪 > 企鹅 > 普通
    std::priority_queue<int> tempQueue = m_npcInventory;
    bool hasYeti = false;
    bool hasPenguin = false;

    while (!tempQueue.empty()) {
        int npcId = tempQueue.top();
        tempQueue.pop();

        if (npcId == 2) { // YetiNPC::ID
            hasYeti = true;
            break;
        } else if (npcId == 1) { // PenguinNPC::ID
            hasPenguin = true;
        }
    }

    if (hasYeti) {
        m_isRidingYeti = true;
        if (m_yetiForm == NPCForm::Normal) {
            m_yetiForm = NPCForm::YetiForm1; // 默认为形态1
        }
        applyNPCForm(m_yetiForm);
        DEBUG_LOG("Player maintaining Yeti form due to inventory");
    } else if (hasPenguin) {
        m_isRidingYeti = false;
        m_yetiForm = NPCForm::Normal;
        applyNPCForm(NPCForm::Penguin);
        DEBUG_LOG("Player maintaining Penguin form due to inventory");
    } else {
        m_isRidingYeti = false;
        m_yetiForm = NPCForm::Normal;
        resetToNormalForm();
        DEBUG_LOG("Player reverting to normal form - no supported NPCs in inventory");
    }
}

// === 雪怪形态管理实现 ===

bool Player::isRidingYeti() const {
    return m_isRidingYeti;
}

Player::NPCForm Player::getYetiForm() const {
    return m_yetiForm;
}

void Player::transformYetiForm() {
    if (m_isRidingYeti && m_yetiForm == NPCForm::YetiForm1) {
        m_yetiForm = NPCForm::YetiForm2;
        applyNPCForm(NPCForm::YetiForm2);
        DEBUG_LOG("Yeti transformed from Form1 to Form2 due to collision");
    }
}

bool Player::canFlip() const {
    if (m_currentForm == NPCForm::Normal || m_currentForm == NPCForm::Penguin) {
        return true;
    } else if (m_currentForm == NPCForm::YetiForm1) {
        return false; // 雪怪形态1不能空翻
    } else if (m_currentForm == NPCForm::YetiForm2) {
        return true;  // 雪怪形态2可以空翻
    }
    return true;
}

// === 企鹅携带系统实现 ===

void Player::addPenguinToCarry(int penguinId) {
    NPCFormModifiers currentModifiers = getFormModifiers(m_currentForm);
    if (m_penguinCarryInventory.size() < currentModifiers.penguinCarryCapacity) {
        m_penguinCarryInventory.push(penguinId);
        DEBUG_LOG(QString("Added penguin to carry inventory: %1, total carried: %2")
                  .arg(penguinId).arg(m_penguinCarryInventory.size()));
    } else {
        DEBUG_LOG("Cannot carry more penguins - capacity reached");
    }
}

bool Player::consumePenguinForDamageResistance() {
    if (m_penguinCarryInventory.empty()) {
        DEBUG_LOG("No carried penguins available for damage resistance");
        return false;
    }

    // 丢弃一只企鹅到地面
    dropCarriedPenguin();

    // 触发拾取冷却，防止立即重新拾取丢弃的企鹅
    startNPCPickupCooldown();

    DEBUG_LOG(QString("Consumed carried penguin for damage resistance, remaining: %1, pickup cooldown activated")
              .arg(m_penguinCarryInventory.size()));
    return true;
}

void Player::dropCarriedPenguin() {
    if (m_penguinCarryInventory.empty()) {
        DEBUG_LOG("No carried penguins to drop");
        return;
    }

    if (!m_terrainGenerator) {
        DEBUG_LOG("TerrainGenerator not available for dropping carried penguin");
        return;
    }

    // 移除一只携带的企鹅
    m_penguinCarryInventory.pop();

    // 在玩家位置生成企鹅
    QPointF spawnPosition = pos();
    qreal terrainHeight = m_terrainGenerator->getTerrainHeight(spawnPosition.x());
    spawnPosition.setY(terrainHeight);

    std::unique_ptr<NPCEntity> newPenguin = NPCFactory::createPenguinNPC(spawnPosition);
    if (newPenguin) {
        // 将企鹅添加到场景和地形生成器
        if (scene()) {
            scene()->addItem(newPenguin.get());
        }

        // 注册到物理系统
        PhysicsSystem::instance().registerObject(newPenguin.get());

        // 添加到地形生成器的NPC列表
        m_terrainGenerator->m_npcs.append(newPenguin.release());

        DEBUG_LOG(QString("Dropped carried penguin at position (%1, %2), remaining carried: %3")
                  .arg(spawnPosition.x()).arg(spawnPosition.y()).arg(m_penguinCarryInventory.size()));

        // 启动拾取冷却
        startNPCPickupCooldown();

        // 触发玩家NPC状态更新信号
        emit updatePlayerNPC();
    } else {
        DEBUG_LOG("Failed to create penguin NPC for dropping");
    }
}

int Player::getCarriedPenguinCount() const {
    return static_cast<int>(m_penguinCarryInventory.size());
}

// 空翻加速进度条更新函数
void Player::updateFlipBoostProgress() {
    if (!m_isFlipBoosting) {
        return; // 如果没有空翻加速，不需要更新进度
    }

    // 计算剩余时间
    int remainingTime = m_flipBoostTimer->remainingTime();

    // 计算进度（0.0 表示刚开始，1.0 表示即将结束）
    qreal progress = 1.0 - (static_cast<qreal>(remainingTime) / static_cast<qreal>(FLIP_BOOST_TIME_MS));

    // 确保进度在有效范围内
    progress = qBound(0.0, progress, 1.0);

    // 发出进度更新信号
    emit flipBoostChanged(true, progress);
}

qreal Player::getInitialMoveSpeed() {
    return initialMoveSpeed;
}







