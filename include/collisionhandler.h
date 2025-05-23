#pragma once

#include <QObject>
#include "physical.h"
#include "terraingenerator.h"
#include "player.h"
#include "rockentity.h"

class CollisionHandler : public QObject
{
    Q_OBJECT

public:
    explicit CollisionHandler(TerrainGenerator* terrainGenerator, QObject* parent = nullptr);
    ~CollisionHandler() = default;

    // 处理物理对象与地形的碰撞
    void handlePhysicsObjectCollision(IPhysicsObject* obj, QList<IPhysicsObject*>& objectsToDelete);
    
    // 更新地形生成器引用
    void updateTerrainGenerator(TerrainGenerator* terrainGenerator);

private:
    // 计算动态地面检测容差
    qreal calculateGroundTolerance(qreal speed, qreal slope, qreal forwardSlope, qreal verticalSpeed);

    // 判断是否应该保持着地状态
    bool shouldMaintainGrounded(bool currentlyGrounded, qreal slope, qreal verticalSpeed, qreal horizontalSpeed);

    // 判断是否应该起飞
    bool shouldTakeoff(qreal backSlope, qreal currentSlope, qreal forwardSlope, qreal speed);

    // 处理起飞/飞跃效果
    void handleTakeoff(IPhysicsObject* obj, qreal slope, qreal speed);

    // 更新斜坡力
    void updateSlopeForce(IPhysicsObject* obj, qreal slope, qreal speed);

    // 更新实体旋转
    void updateEntityRotation(BasePhysicsEntity* entity, bool onGround, qreal slope);

    // 处理玩家与石头的碰撞
    void handlePlayerRockCollision(Player* player, QList<IPhysicsObject*>& objectsToDelete);

    // 地形生成器引用
    TerrainGenerator* m_terrainGenerator;
};
