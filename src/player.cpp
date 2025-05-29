#include "player.h"
#include <QBrush>
#include <QPen>
#include "debuglogger.h"
#include <qpainter.h>

#include "terraingenerator.h"

// 定义静态常量
const qreal Player::MAX_LANDING_ANGLE_DEVIATION = 45.0; // 45度最大偏差

// 定义NPC拾取冷却时间（毫秒）
const int PICKUP_COOLDOWN_MS = 3000; // 3秒冷却

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
      keyRight(false),
      keySpace(false),
      rotateSpeed(3),
      m_moveSpeed(500),
      m_jumpForce(-300),
      m_takeoffRotation(0.0),
      m_flipRotation(0.0),
      m_cumulativeRotation(0.0),
      m_lastFrameRotation(0.0),
      m_fallRecoveryTimer(this),
      m_animationFrames(),
      m_currentFrame(0),
      m_animationTimer(this),
      m_animationLoaded(false),
      m_imageScaleFactor(1),
      m_npcCarryManager(),
      m_baseSpeed(500),
      m_baseJumpForce(-300),
      m_maxCarryCount(5),
      m_isPickupCooldown(false),
      m_pickupCooldownTimer(this)
{

    setZValue(-2);

    // 设置玩家外观
    setBrush(QBrush(Qt::red));
    setPen(QPen(Qt::black, 2));

    // 设置实体类型
    setEntityType(EntityType::Player);

    // 允许接收键盘焦点
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFocus();    // 初始化摔倒恢复计时器
    connect(&m_fallRecoveryTimer, &QTimer::timeout, this, &Player::onFallRecoveryTimeout);
    m_fallRecoveryTimer.setSingleShot(true);
    
    // 初始化拾取冷却计时器
    connect(&m_pickupCooldownTimer, &QTimer::timeout, this, &Player::onPickupCooldownEnd);
    m_pickupCooldownTimer.setSingleShot(true);
    
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
    
    // 检查是否有NPC提供碰撞抵抗
    if (isCollisionResisted()) {
        DEBUG_LOG("Rock collision resisted by NPC effect!");
        // 失去一个NPC并进入冷却，但不摔倒
        loseNPCAndCooldown();
        return;
    }
    
    // 没有抵抗能力，玩家摔倒
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
    Q_UNUSED(GTerrainGenerator);  // 标记参数未使用
    
    if (is_fallen) {
        // 摔倒姿势 - 侧躺
        setRotation(90); // 简单的90度侧躺姿势
        return;
    }

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

void Player::onPickupCooldownEnd() {
    m_isPickupCooldown = false;
    DebugLogger::instance()->log("Player pickup cooldown ended");
}

bool Player::canResistFall(qreal angleDeviation) const {
    Q_UNUSED(angleDeviation);  // 标记参数未使用
    // 检查是否有携带的NPC提供碰撞抵抗
    return isCollisionResisted();
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

// === NPCCarryManager 实现 ===

void NPCCarryManager::addNPC(NPCEntity* npc) {
    if (!npc) {
        DebugLogger::instance()->log("Cannot add null NPC to carry manager");
        return;
    }
    
    if (m_npcSet.find(npc) != m_npcSet.end()) {
        DebugLogger::instance()->log("NPC already in carry list - skipping");
        return; // NPC已经携带
    }
    
    DebugLogger::instance()->log(QString("Adding NPC with priority %1 to carry list")
                                .arg(npc->getPriority()));
    
    // 标记NPC为已携带状态
    npc->setCarried(true);
    
    // 添加到集合和优先级队列
    m_npcSet.insert(npc);
    m_carriedNPCs.emplace(npc, npc->getCarryEffect(), npc->getPriority());
    
    DebugLogger::instance()->log(QString("Successfully added NPC, current carry count: %1")
                                .arg(m_carriedNPCs.size()));
}

bool NPCCarryManager::removeNPC(NPCEntity* npc) {
    if (!npc || m_npcSet.find(npc) == m_npcSet.end()) {
        DebugLogger::instance()->log("Cannot remove NPC: not found in carry list");
        return false; // NPC为空或未携带
    }
    
    DebugLogger::instance()->log(QString("Removing NPC with priority %1 from carry list")
                                .arg(npc->getPriority()));
    
    // 从集合移除
    m_npcSet.erase(npc);
    
    // 标记NPC为未携带状态
    npc->setCarried(false);
    
    // 从优先级队列重建（因为priority_queue不支持直接删除中间元素）
    std::priority_queue<CarriedNPC> tempQueue;
    int removedCount = 0;
    
    while (!m_carriedNPCs.empty()) {
        CarriedNPC carried = m_carriedNPCs.top();
        m_carriedNPCs.pop();
        if (carried.npc != npc) {
            tempQueue.push(carried);
        } else {
            removedCount++;
        }
    }
    m_carriedNPCs = std::move(tempQueue);
    
    DebugLogger::instance()->log(QString("Removed %1 instances of NPC, current carry count: %2")
                                .arg(removedCount).arg(m_carriedNPCs.size()));
    
    return removedCount > 0;
}

NPCEntity* NPCCarryManager::getHighestPriorityNPC() const {
    if (m_carriedNPCs.empty()) {
        return nullptr;
    }
    return m_carriedNPCs.top().npc;
}

NPCEntity* NPCCarryManager::getLowestPriorityNPC() const {
    if (m_carriedNPCs.empty()) {
        return nullptr;
    }
    
    // 由于优先级队列是最大堆，最低优先级的元素在底部
    // 我们需要遍历找到最低优先级
    auto tempQueue = m_carriedNPCs;
    CarriedNPC lowest = tempQueue.top();
    tempQueue.pop();
    
    while (!tempQueue.empty()) {
        CarriedNPC current = tempQueue.top();
        tempQueue.pop();
        if (current.priority < lowest.priority) {
            lowest = current;
        }
    }
    
    return lowest.npc;
}

NPCCarryEffect NPCCarryManager::getHighestPriorityEffect() const {
    if (m_carriedNPCs.empty()) {
        return NPCCarryEffect(); // 返回默认效果
    }
    return m_carriedNPCs.top().effect;
}

NPCCarryEffect NPCCarryManager::getTotalEffect() const {
    if (m_carriedNPCs.empty()) {
        return NPCCarryEffect();
    }
    
    // 简单实现：只使用最高优先级的效果
    // 可以根据需要扩展为叠加多个效果
    return getHighestPriorityEffect();
}

void NPCCarryManager::clear() {
    // 将所有NPC标记为未携带
    for (NPCEntity* npc : m_npcSet) {
        if (npc) {
            npc->setCarried(false);
        }
    }
    
    // 清空数据结构
    m_npcSet.clear();
    while (!m_carriedNPCs.empty()) {
        m_carriedNPCs.pop();
    }
    
    DEBUG_LOG("清空所有携带的NPC");
}

bool NPCCarryManager::isCarrying(NPCEntity* npc) const {
    return m_npcSet.find(npc) != m_npcSet.end();
}

std::vector<NPCEntity*> NPCCarryManager::getAllCarriedNPCs() const {
    std::vector<NPCEntity*> result;
    std::priority_queue<CarriedNPC> tempQueue = m_carriedNPCs;
    
    while (!tempQueue.empty()) {
        result.push_back(tempQueue.top().npc);
        tempQueue.pop();
    }
    
    return result;
}

// === Player NPC携带系统实现 ===

bool Player::pickupNPC(NPCEntity* npc) {
    if (!canPickupNPC() || !npc || !npc->isCarriable() || npc->isCarried()) {
        DebugLogger::instance()->log("Cannot pickup NPC: failed preconditions");
        return false;
    }
      
    if (getCarriedNPCCount() >= m_maxCarryCount) {
        DebugLogger::instance()->log("Cannot pickup NPC: carry limit reached");
        return false;
    }
    
    // 先设置NPC状态，防止重复拾取
    npc->setCarried(true);
    
    // 添加到管理器
    m_npcCarryManager.addNPC(npc);
    
    // === 从场景中移除NPC（但不删除对象） ===
    if (npc->scene()) {
        npc->scene()->removeItem(npc);
        DebugLogger::instance()->log("Removed NPC from scene after pickup");
    }
    
    // 从物理系统注销
    PhysicsSystem::instance().unregisterObject(npc);
    
    // 应用携带效果
    applyCarryEffects();
      
    DebugLogger::instance()->log(QString("Successfully picked up NPC with priority %1, carrying %2 NPCs")
                                .arg(npc->getPriority()).arg(getCarriedNPCCount()));
    
    return true;
}

bool Player::dropNPC(NPCEntity* npc) {
    if (!npc || !m_npcCarryManager.isCarrying(npc)) {
        return false;
    }
    
    // 从管理器移除
    bool removed = m_npcCarryManager.removeNPC(npc);
    
    if (removed) {
        // 重置NPC状态
        npc->setCarried(false);
        
        // 重新应用携带效果
        applyCarryEffects();
          DebugLogger::instance()->log(QString("Dropped NPC with priority %1").arg(npc->getPriority()));
    }
    
    return removed;
}

void Player::loseNPCAndCooldown() {
    // 失去最低优先级的NPC
    NPCEntity* lowestPriorityNPC = m_npcCarryManager.getLowestPriorityNPC();
    
    if (lowestPriorityNPC) {
        QPointF respawnPos = pos(); // 在玩家位置重生
        NPCEntity::NPCType npcType = lowestPriorityNPC->getNPCType();
        
        DebugLogger::instance()->log(QString("About to lose NPC with priority %1, current count: %2")
                                    .arg(lowestPriorityNPC->getPriority())
                                    .arg(getCarriedNPCCount()));
        
        // 移除NPC - 这会真正从携带列表中删除
        bool removed = dropNPC(lowestPriorityNPC);
        
        if (removed) {
            // 发射信号，让GameScene处理重生
            emit npcDropped(npcType, respawnPos);
            DebugLogger::instance()->log(QString("Lost NPC due to collision, remaining: %1")
                                        .arg(getCarriedNPCCount()));
        } else {
            DebugLogger::instance()->log("Failed to remove NPC from carry list!");
        }
    } else {
        DebugLogger::instance()->log("No NPC to lose - carry list might be empty");
    }
    
    // 进入冷却状态
    m_isPickupCooldown = true;
    m_pickupCooldownTimer.start(PICKUP_COOLDOWN_MS);
      DebugLogger::instance()->log(QString("Pickup cooldown started for %1ms").arg(PICKUP_COOLDOWN_MS));
}

bool Player::isCollisionResisted() const {
    int carriedCount = getCarriedNPCCount();
    bool canResist = carriedCount > 0;
    
    DebugLogger::instance()->log(QString("Collision resistance check: carrying %1 NPCs, can resist: %2")
                                .arg(carriedCount).arg(canResist ? "YES" : "NO"));
    
    return canResist;
}

void Player::applyCarryEffects() {
    NPCCarryEffect totalEffect = getCurrentCarryEffect();
    
    // 应用速度倍率
    m_moveSpeed = m_baseSpeed * totalEffect.speedMultiplier;
    
    // 应用跳跃力倍率
    m_jumpForce = m_baseJumpForce * totalEffect.jumpForceMultiplier;
      DebugLogger::instance()->log(QString("Applied carry effects: speed=%1, jump=%2")
                     .arg(m_moveSpeed).arg(m_jumpForce));
}

// === NPC携带查询方法实现 ===

int Player::getCarriedNPCCount() const {
    return m_npcCarryManager.getCarriedCount();
}

NPCEntity* Player::getHighestPriorityNPC() const {
    return m_npcCarryManager.getHighestPriorityNPC();
}

NPCCarryEffect Player::getCurrentCarryEffect() const {
    return m_npcCarryManager.getTotalEffect();
}

bool Player::isCarryingNPC(NPCEntity* npc) const {
    return m_npcCarryManager.isCarrying(npc);
}

std::vector<NPCEntity*> Player::getAllCarriedNPCs() const {
    return m_npcCarryManager.getAllCarriedNPCs();
}

bool Player::canPickupNPC() const {
    return !m_isPickupCooldown;
}

QString Player::getCarryStatusString() const {
    return QString("Carrying %1/%2 NPCs").arg(getCarriedNPCCount()).arg(m_maxCarryCount);
}