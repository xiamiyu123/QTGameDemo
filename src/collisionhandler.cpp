#include "collisionhandler.h"
#include "debuglogger.h"

CollisionHandler::CollisionHandler(TerrainGenerator* terrainGenerator, QObject* parent)
    : QObject(parent)
    , m_terrainGenerator(terrainGenerator)
{
}

// 更新地形生成器引用
void CollisionHandler::updateTerrainGenerator(TerrainGenerator* terrainGenerator)
{
    m_terrainGenerator = terrainGenerator;
}

// 处理物理对象与地形的碰撞
void CollisionHandler::handlePhysicsObjectCollision(IPhysicsObject* obj, QList<IPhysicsObject*>& objectsToDelete)
{
    if (!obj || !m_terrainGenerator) {
        DEBUG_LOG("CollisionHandler::handlePhysicsObjectCollision - null pointer detected");
        return;
    }

    // 获取物体信息
    QRectF objRect = obj->boundingRect();
    QPointF objPos = obj->position();
    qreal objCenterX = objPos.x() + objRect.width() / 2;

    // 多点采样获取地形信息 - 更好地适应不规则地形
    qreal centerTerrain = m_terrainGenerator->getTerrainHeight(objCenterX);
    qreal leftTerrain = m_terrainGenerator->getTerrainHeight(objCenterX - objRect.width() * 0.4);
    qreal rightTerrain = m_terrainGenerator->getTerrainHeight(objCenterX + objRect.width() * 0.4);
    qreal terrainHeight = qMin(qMin(leftTerrain, centerTerrain), rightTerrain);

    // 获取当前和预测的地形斜率
    qreal terrainSlope = m_terrainGenerator->getTerrainSlope(objCenterX);
    qreal backSlope = m_terrainGenerator->getTerrainSlope(objCenterX - objRect.width() * 0.6);
    qreal forwardSlope = m_terrainGenerator->getTerrainSlope(objCenterX + objRect.width() * 0.6);

    // 获取物体速度
    QPointF velocity = obj->velocity();
    qreal horizontalSpeed = qAbs(velocity.x());
    qreal verticalSpeed = velocity.y();    // 获取实体对象和实际底部位置 - 统一使用局部坐标
    BasePhysicsEntity* entity = dynamic_cast<BasePhysicsEntity*>(obj);

    qreal actualObjBottom;
    if (entity) {
        // 使用局部坐标系统，与 objPos 保持一致
        actualObjBottom = objPos.y() + objRect.height();
    }
    else {
        actualObjBottom = objPos.y() + objRect.height();
    }

    // 动态地面检测容差 - 核心改进
    qreal groundTolerance = calculateGroundTolerance(horizontalSpeed, terrainSlope, forwardSlope, verticalSpeed);

    // 添加玩家对象的检测
    Player* player = dynamic_cast<Player*>(obj); // 注意：这里的 player 变量名可能会与函数参数 obj 混淆，但它是局部变量

    bool wasOnGround = obj->isOnGround();
    //如果是npc且高度低于地形
    if( entity && entity->getEntityType() == IPhysicsObject::EntityType::NPC &&
        actualObjBottom >= terrainHeight) {
        int i = 0;
    }

    // 主要碰撞逻辑 (地面碰撞等)
    if (actualObjBottom >= terrainHeight) {  // 已穿透地面
        // 校正位置
        qreal dy_adjust = terrainHeight - actualObjBottom;
        obj->setPosition(QPointF(objPos.x(), objPos.y() + dy_adjust));

        // 设置为着地状态
        if (!wasOnGround) {
            obj->setOnGround(true);
            if (player) { // 如果 obj 是玩家
                qreal terrainAngle = qRadiansToDegrees(qAtan(terrainSlope));
                player->checkLanding(terrainAngle);            }
            obj->setVelocity(QPointF(velocity.x(), 0));
            DEBUG_LOG(QString("落地: 地形高度 = %1 角色底部 = %2").arg(terrainHeight).arg(actualObjBottom));
        }
        updateSlopeForce(obj, terrainSlope, horizontalSpeed);
    }
    else if (actualObjBottom + groundTolerance >= terrainHeight) {  // 接近地面
        // 判断是否应该保持着地
        bool shouldStayGrounded = shouldMaintainGrounded(wasOnGround, terrainSlope, verticalSpeed, horizontalSpeed);
        if (shouldStayGrounded) {
            // 校正位置 - 平滑吸附到地面
            qreal snapFactor = 1;  // 吸附强度
            qreal dy_adjust = (terrainHeight - actualObjBottom) * snapFactor;
            obj->setPosition(QPointF(objPos.x(), objPos.y() + dy_adjust));

            if (!wasOnGround) {
                obj->setOnGround(true);
                if (player) { // 如果 obj 是玩家
                    qreal terrainAngle = qRadiansToDegrees(qAtan(terrainSlope));
                    player->checkLanding(terrainAngle);
                }
                obj->setVelocity(QPointF(velocity.x(), 0));
                DEBUG_LOG(QString("靠近地面落地: 地形高度 = %1 角色底部 = %2").arg(terrainHeight).arg(actualObjBottom));
            }
            updateSlopeForce(obj, terrainSlope, horizontalSpeed);
        }
        else if (wasOnGround && shouldTakeoff(backSlope, terrainSlope, forwardSlope, horizontalSpeed)) {
            handleTakeoff(obj, terrainSlope, horizontalSpeed);
        }  else if (wasOnGround) {
            obj->setOnGround(false);
            obj->setSlopeSlideSpeed(0);
            if (player) { // 如果 obj 是玩家
                player->notifyTakeoff();
            }
        }
    }
    else {  // 明显离开地面
        if (wasOnGround) {
            obj->setOnGround(false);
            obj->setSlopeSlideSpeed(0);
            if (player) { // 如果 obj 是玩家
                player->notifyTakeoff();
            }
        }
    }

    // 更新物体姿态 - 考虑摔倒状态
    if (entity) {
        Player* asPlayer = dynamic_cast<Player*>(entity); // 检查 entity 是否为 Player
        if (!asPlayer || !asPlayer->isFallen()) {
            updateEntityRotation(entity, obj->isOnGround(), terrainSlope);
        }
    }    // 玩家与石头碰撞检测 - 仅当当前 obj 是玩家时执行
    if (player) {
        handlePlayerRockCollision(player, objectsToDelete);
        handlePlayerNPCCollision(player, objectsToDelete);  // 新增：NPC碰撞检测
    }

    // 在调用 obj->setOnGround(true); 和 obj->setPosition(...) 之后
    if (obj->isOnGround()) {
        BasePhysicsEntity* entity = dynamic_cast<BasePhysicsEntity*>(obj);
        if (entity) { // 确保是我们的实体类型
            qreal currentCenterX = entity->position().x() + entity->boundingRect().width() / 2;
            qreal authoritativeTerrainHeight = m_terrainGenerator->getTerrainHeight(currentCenterX); // 获取NPC中心正下方的地形高度
            qreal expectedTopY = authoritativeTerrainHeight - entity->boundingRect().height();
            
            // 如果当前Y坐标与期望Y坐标有明显偏差，则强制校正
            if (qAbs(entity->position().y() - expectedTopY) > 50) { // 0.1 是一个小的容差值
                entity->setPosition(QPointF(entity->position().x(), expectedTopY));
                // 可以添加日志记录这次强制校正
                // DebugLogger::instance().log(QString("CollisionHandler: NPC ID %1 re-corrected to Y %2 based on center terrain %3")
                //    .arg(entity->getEntityType() == IPhysicsObject::EntityType::NPC ? static_cast<NPCEntity*>(entity)->class_id() : 0) // 假设有class_id
                //    .arg(expectedTopY).arg(authoritativeTerrainHeight));
            }
        }
    }
}

// 处理玩家与石头的碰撞
void CollisionHandler::handlePlayerRockCollision(Player* player, QList<IPhysicsObject*>& objectsToDelete)
{
    // 防御性检查
    if (!player || !m_terrainGenerator) {
        return;
    }
    
    // 确保玩家和场景对象都有效
    if (!player->scene()) {
        return;
    }
    
    // 先进行单线程循环处理
    for (int i = m_terrainGenerator->m_rocks.size() - 1; i >= 0; --i) {
        RockEntity* rock = m_terrainGenerator->m_rocks.at(i);
        if (!rock || !rock->scene()) {
            continue;
        }

        // 检查石头是否已在本帧中被标记为删除 (安全措施)
        bool rockAlreadyMarkedForDeletion = false;
        for (IPhysicsObject* deletedObj : objectsToDelete) {
            if (rock == deletedObj) {
                rockAlreadyMarkedForDeletion = true;
                break;
            }
        }
        if (rockAlreadyMarkedForDeletion) {
            continue;
        }

        // 碰撞检测前确保两个对象都有效且在同一场景中
        if (player->scene() == rock->scene() && player->collidesWithItem(rock)) {
            player->checkHitRock(rock); // 调用修改后的方法，仅处理玩家状态

            // 从场景中移除石头
            if (rock->scene()) {
                rock->scene()->removeItem(rock);
            }
            
            // 从物理系统中注销石头
            PhysicsSystem::instance().unregisterObject(rock);
            
            // 从地形生成器的石头列表中移除
            m_terrainGenerator->m_rocks.removeAt(i);

            // 将石头添加到本帧的待删除列表
            if (!objectsToDelete.contains(rock)) {
                objectsToDelete.append(rock);
            }
            
            break; // 处理完一次碰撞即可
        }    }
}

// 计算动态地面检测容差
qreal CollisionHandler::calculateGroundTolerance(qreal speed, qreal slope, qreal forwardSlope, qreal verticalSpeed) {
    // 基础容差
    qreal baseTolerance = 4.0;

    // 速度调整因子 - 高速时增加容差
    qreal speedFactor = qMin(1.0 + speed / 300.0, 2.5);

    // 坡度调整因子
    qreal slopeFactor = 1.0;
    if (slope < -0.3) {  // 陡下坡
        slopeFactor = 1.3 - slope;  // 更陡的斜坡，更大的容差
    }

    // 前方斜率预测 - 即将下陡坡时提前增加容差
    if (forwardSlope < slope && forwardSlope < -0.3) {
        slopeFactor *= 1.2;
    }

    // 垂直速度调整 - 跳跃时减小容差
    if (verticalSpeed < -30) {
        return 2.0;  // 跳跃时最小容差
    }

    return baseTolerance * speedFactor * slopeFactor;
}

// 判断是否应该保持着地状态
bool CollisionHandler::shouldMaintainGrounded(bool currentlyGrounded, qreal slope, qreal verticalSpeed, qreal horizontalSpeed) {
    // 已经在地面上，增加"粘性"避免轻微抖动
    if (currentlyGrounded) {
        return true;
    }

    // 明显向下运动时应着地
    if (verticalSpeed > 5) {
        return true;
    }

    // 高速下坡时更容易保持着地
    if (slope < -0.2 && horizontalSpeed > 120) {
        return true;
    }

    return false;
}

// 判断是否应该起飞
bool CollisionHandler::shouldTakeoff(qreal backSlope, qreal currentSlope, qreal forwardSlope, qreal speed) {
    // 只在速度足够时考虑起飞
    if (speed < 100) {
        return false;
    }

    // 从上坡过渡到下坡(山顶/跳台效果)
    if (backSlope > 0.2 && currentSlope < -0.2) {
        return true;
    }

    // 急剧下坡
    if (currentSlope < -0.5 && speed > 200) {
        return true;
    }

    return false;
}

// 处理起飞/飞跃效果
void CollisionHandler::handleTakeoff(IPhysicsObject* obj, qreal slope, qreal speed) {
    // 计算起飞的垂直速度
    qreal takeoffForce = -slope * speed * 0.3;

    // 限制最小和最大起飞力
    takeoffForce = qBound(-200.0, takeoffForce, -50.0);

    // 高速时给予额外的飞跃效果
    if (speed > 250) {
        takeoffForce *= 1.2;
    }

    // 设置为非着地状态并应用垂直速度
    obj->setOnGround(false);
    obj->setSlopeSlideSpeed(0);
    obj->setVelocity(QPointF(obj->velocity().x(), takeoffForce));

    // 检查是否为Player对象并调用离地通知
    Player* player = dynamic_cast<Player*>(obj);
    if (player) {
        player->notifyTakeoff();
    }
}

// 更新斜坡力
void CollisionHandler::updateSlopeForce(IPhysicsObject* obj, qreal slope, qreal speed) {
    // 斜坡力阈值和系数
    const qreal slopeThreshold = 0.08;
    const qreal downhillFactor = 300.0;
    const qreal uphillFactor = 200.0;

    qreal slopeForce = 0;

    if (qAbs(slope) > slopeThreshold) {
        // 基础斜坡力
        slopeForce = slope * (slope < 0 ? downhillFactor : uphillFactor);

        // 高速下坡时增加力
        if (slope < -0.2 && speed > 150) {
            slopeForce *= (1.0 + speed / 500.0);
        }
    }

    // 限制最大斜坡力
    slopeForce = qBound(-250.0, slopeForce, 150.0);
    obj->setSlopeSlideSpeed(slopeForce);
}

// 更新实体旋转
void CollisionHandler::updateEntityRotation(BasePhysicsEntity* entity, bool onGround, qreal slope) {
    if (!entity) return;

    // 检查是否为玩家且是否摔倒
    Player* player = dynamic_cast<Player*>(entity);
    if (player && player->isFallen()) {
        // 摔倒状态下不更新旋转，由玩家类自己控制
        return;
    }

    // 只有在地面上才跟随地形旋转
    if (onGround) {
        qreal targetAngle = qAtan(slope) * 180.0 / M_PI;
        entity->setRotation(targetAngle);
    }
    // 空中的旋转逻辑由各实体类自行控制
}

// 处理玩家与NPC的碰撞
void CollisionHandler::handlePlayerNPCCollision(Player* player, QList<IPhysicsObject*>& objectsToDelete)
{
    // 防御性检查
    if (!player || !m_terrainGenerator) {
        return;
    }
    
    // 确保玩家和场景对象都有效
    if (!player->scene()) {
        return;    }
    
    // 检查玩家是否还有库存空间
    if (player->getInventorySize() >= 10) { // MAX_INVENTORY_SIZE
        return; // 库存已满，无法拾取更多NPC
    }
    
    // 检查拾取冷却状态
    if (!player->canPickupNPC()) {
        return; // 拾取冷却中，不能拾取NPC
    }
    
    // 获取玩家局部坐标 - 与物理系统保持一致
    QPointF playerPos = player->position();
    QRectF playerRect = player->boundingRect();
    QPointF playerCenter = QPointF(playerPos.x() + playerRect.width()/2, 
                                   playerPos.y() + playerRect.height()/2);
      // 遍历所有NPC进行碰撞检测
    for (int i = m_terrainGenerator->m_npcs.size() - 1; i >= 0; --i) {
        NPCEntity* npc = m_terrainGenerator->m_npcs.at(i);
        if (!npc || !npc->scene()) {
            continue;
        }

        // 使用局部坐标计算NPC与玩家的距离，与物理系统保持一致
        QPointF npcPos = npc->position();
        QRectF npcRect = npc->boundingRect();
        QPointF npcCenter = QPointF(npcPos.x() + npcRect.width()/2, 
                                    npcPos.y() + npcRect.height()/2);
        
        qreal distance = QLineF(playerCenter, npcCenter).length();        // 如果距离在500以内，输出调试信息（包含坐标系对比）
        if (distance <= 500.0) {
            // 获取场景坐标用于对比
            QRectF npcSceneRect = npc->sceneBoundingRect();
            QRectF playerSceneRect = player->sceneBoundingRect();
            QPointF npcSceneCenter = npcSceneRect.center();
            QPointF playerSceneCenter = playerSceneRect.center();
            
            // 计算坐标差异
            QPointF npcCoordDiff = npcSceneCenter - npcCenter;
            QPointF playerCoordDiff = playerSceneCenter - playerCenter;
            // 输出调试信息
             DEBUG_LOG(QString("NPC Status - ID: %1, Distance: %2")
                       .arg(npc->class_id())
                       .arg(distance, 0, 'f', 1));
            //输出npc是否着地
            DEBUG_LOG(QString("NPC OnGround: %1").arg(npc->isOnGround() ? "true" : "false"));
            // 输出NPC的局部坐标
            DEBUG_LOG(QString("NPC Local Coords - Center: (%1, %2)")
                      .arg(npcCenter.x(), 0, 'f', 1)
                      .arg(npcCenter.y(), 0, 'f', 1));
            //输出该点地面高度
            DEBUG_LOG(QString("Terrain Height at NPC: %1").arg(m_terrainGenerator->getTerrainHeight(npcCenter.x())));
            // DEBUG_LOG(QString("  Local Coords - NPC: (%1, %2), Player: (%3, %4)")
            //           .arg(npcCenter.x(), 0, 'f', 1)
            //           .arg(npcCenter.y(), 0, 'f', 1)
            //           .arg(playerCenter.x(), 0, 'f', 1)
            //           .arg(playerCenter.y(), 0, 'f', 1));
            // DEBUG_LOG(QString("  Scene Coords - NPC: (%1, %2), Player: (%3, %4)")
            //           .arg(npcSceneCenter.x(), 0, 'f', 1)
            //           .arg(npcSceneCenter.y(), 0, 'f', 1)
            //           .arg(playerSceneCenter.x(), 0, 'f', 1)
            //           .arg(playerSceneCenter.y(), 0, 'f', 1));
            // DEBUG_LOG(QString("  Coord Diff - NPC: (%1, %2), Player: (%3, %4)")
            //           .arg(npcCoordDiff.x(), 0, 'f', 1)
            //           .arg(npcCoordDiff.y(), 0, 'f', 1)
            //           .arg(playerCoordDiff.x(), 0, 'f', 1)
            //           .arg(playerCoordDiff.y(), 0, 'f', 1)
            //           .arg(playerCoordDiff.y(), 0, 'f', 1));
            // DEBUG_LOG(QString("  OnGround: NPC=%1, Player=%2")
            //           .arg(npc->isOnGround() ? "true" : "false")
            //           .arg(player->isOnGround() ? "true" : "false"));
        }

        // 检查NPC是否已在本帧中被标记为删除
        bool npcAlreadyMarkedForDeletion = false;
        for (IPhysicsObject* deletedObj : objectsToDelete) {
            if (npc == deletedObj) {
                npcAlreadyMarkedForDeletion = true;
                break;
            }
        }
        if (npcAlreadyMarkedForDeletion) {
            continue;
        }        // 检测是否应该拾取NPC
        bool shouldPickup = false;
        QString collisionMethod = "";
        
        if (player->scene() == npc->scene() && player->collidesWithItem(npc)) {
            shouldPickup = true;
            collisionMethod = "Qt collision";
        }
        
        if (shouldPickup) {
            // 玩家拾取NPC
            player->pickupNPC(npc);

            // 从场景中移除NPC
            if (npc->scene()) {
                npc->scene()->removeItem(npc);
            }
            
            // 从物理系统中注销NPC
            PhysicsSystem::instance().unregisterObject(npc);
            
            // 从地形生成器的NPC列表中移除
            m_terrainGenerator->m_npcs.removeAt(i);

            // 将NPC添加到本帧的待删除列表
            if (!objectsToDelete.contains(npc)) {
                objectsToDelete.append(npc);
            }
            
            DEBUG_LOG(QString("Player picked up NPC ID: %1 using %2 (distance: %3)")
                      .arg(npc->class_id())
                      .arg(collisionMethod)
                      .arg(distance, 0, 'f', 1));
        }
    }
}
