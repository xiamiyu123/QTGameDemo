#include "gamescene.h"
#include <QGraphicsView>
#include <QtSvg>
#include "physical.h"
GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent)
{
    // 设置场景大小（足够大以容纳滚动地形）
    setSceneRect(0, 0, 2000000, 2000000);

    GState = Running; // 初始化游戏状态为运行中
    
    // 创建地形生成器
    GTerrainGenerator = new TerrainGenerator(this, this);
    // 创建并隐藏暂停时显示的文本
    GPauseText = addText("", QFont("Arial", 24));
    GPauseText->setDefaultTextColor(Qt::white);
    GPauseText->setZValue(1000);
    GPauseText->hide();

    // 创建玩家
    Gplayer = new Player();
    addItem(Gplayer);

    // 注册玩家到物理系统
    PhysicsSystem::instance().registerObject(Gplayer);
    
    // 设置游戏循环定时器
    connect(&GTimer, &QTimer::timeout, this, &GameScene::update);
    GTimer.setInterval(16); // 约60fps

    // 设置ui控件
    // 设置暂停按钮
    pauseButton = new QPushButton();
    pauseButton->setIcon(QIcon(":/images/icons/pause.svg"));
    pauseButton->setFocusPolicy(Qt::NoFocus);
    pauseButton->setIconSize(QSize(50, 50));
    connect(pauseButton, &QPushButton::pressed, this, &GameScene::togglePause);
    //隐藏按钮背景
    pauseButton->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    //按钮图片适配大小
    pauseButton->setAttribute(Qt::WA_TranslucentBackground);
}

GameScene::~GameScene()
{
}

void GameScene::initialize()
{
    // 初始化地形
    GTerrainGenerator->initialize();
    
    // 将玩家放置在适当位置
    initialPlayerPosition();
    
    // 启动游戏循环
    GElapsedTimer.start();
    GTimer.start();
}

void GameScene::keyPressEvent(QKeyEvent *event)
{
    // 把键盘事件传递给玩家
    // 处理暂停和继续
    if (event->key() == Qt::Key_P) {
        togglePause();
    }
    if (event->isAutoRepeat()) {
        return; // 忽略自动重复事件
    }
    Gplayer->keyPressEvent(event);
    //QGraphicsScene::keyPressEvent(event);
}

void GameScene::keyReleaseEvent(QKeyEvent *event)
{
    // 把键盘事件传递给玩家
    if (event->isAutoRepeat()) {
        return; // 忽略自动重复事件
    }
    Gplayer->keyReleaseEvent(event);

    //QGraphicsScene::keyReleaseEvent(event);
}

void GameScene::update() {
    // 时间增量16ms
    qreal deltaTime = 16.0f / 1000.0f;

    // 更新物理系统中所有实体的物理状态
    PhysicsSystem::instance().update(deltaTime);

    // 处理所有物理对象的碰撞
    const QList<IPhysicsObject*>& physicsObjects = PhysicsSystem::instance().getPhysicsObjects();
    for (IPhysicsObject* obj : physicsObjects) {
        handlePhysicsObjectCollision(obj);
    }

    // 更新地形生成（基于玩家位置）
    GTerrainGenerator->updateTerrain(Gplayer->x());

    // 让视图跟随玩家
    centerViewOnPlayer();

    // 更新UI控件
    updateUI();
}

// 仅用于初始化时放置玩家
void GameScene::initialPlayerPosition() {

    qreal terrainHeight = GTerrainGenerator->getTerrainHeight(1200);
    Gplayer->setY(terrainHeight - Gplayer->rect().height());
    Gplayer->setX(1200); // 玩家方块偏左一点以更符合滑雪大冒险
    Gplayer->setOnGround(true);
}

void GameScene::centerViewOnPlayer()
{
    if (views().isEmpty()) {
        return;
    }
    
    QGraphicsView *view = views().first();
    
    // 计算视图中间位置
    qreal viewCenterX = Gplayer->x() + 200; // 玩家方块偏左一点以更符合滑雪大冒险
    qreal viewCenterY = Gplayer->y();
    
    // 设置视图中心
    view->centerOn(viewCenterX, viewCenterY);
}

void GameScene::togglePause()
{
    if (GState == Running) {
        GState = Paused;
        GTimer.stop();
        qreal secs = GElapsedTimer.elapsed() / 1000.0;
        GPauseText->setPlainText(QString("游戏已暂停，已进行" + QString::number(secs, 'f', 2) + "秒"));
        //更改暂停按钮图标
        pauseButton->setIcon(QIcon(":/images/icons/play.svg"));
        // 设置文本位置，使其居中
        // 获取视口在场景中的矩形
        if (!views().isEmpty()) {
            QGraphicsView *view = views().last();
            QRectF viewSceneRect = view->mapToScene(view->viewport()->geometry()).boundingRect();
            QRectF textRect = GPauseText->boundingRect();
            // 文字居中到视口
            GPauseText->setPos(
                viewSceneRect.center().x() - textRect.width() / 2,
                viewSceneRect.center().y() - textRect.height() / 2
            );
        }
        GPauseText->show();
    } else {
        GState = Running;
        GPauseText->hide();
        GTimer.start();
        //更改暂停按钮图标
        pauseButton->setIcon(QIcon(":/images/icons/pause.svg"));
    }
}

void GameScene::updateUI()
{
    QGraphicsView *view = views().first();
    // 确保按钮有父对象
    pauseButton->setParent(view->viewport());
    // 固定暂停按钮在右上角，10px 边距
    QRect vp = view->viewport()->rect();
    pauseButton->setGeometry(vp.width() - 50 - 10, 10, 50, 50);
    pauseButton->show();

}

void GameScene::handlePhysicsObjectCollision(IPhysicsObject* obj) {
    // 计算物体中心的x坐标（用于获取地形信息）
    qreal objX = obj->position().x() + obj->boundingRect().width() / 2;
    // 获取物体中心x坐标处的地形高度（地面位置）
    qreal terrainHeight = GTerrainGenerator->getTerrainHeight(objX) + 2;//模拟陷入雪中
    // 获取物体中心x坐标处的地形斜率（用于坡面滑动计算）
    qreal terrainSlope = GTerrainGenerator->getTerrainSlope(objX);

    // 定义地面检测的容差值
    qreal groundTolerance = 6.0;

    BasePhysicsEntity* entity = dynamic_cast<BasePhysicsEntity*>(obj);

    // 设置物体旋转角度以匹配地形斜率
    // 这一步很重要，确保在获取 sceneBoundingRect 之前应用旋转
    if (entity && obj->isOnGround()) {
        qreal angle = qAtan(terrainSlope) * 180.0 / M_PI;
        entity->setRotation(angle);
    }

    // 获取物体旋转后的实际底部 Y 坐标
    qreal actualObjVisualBottomY;
    QRectF sceneBounds; // 用于存储场景包围矩形

    if (entity) {
        // sceneBoundingRect() 返回的是物体在场景坐标系下的包围盒，已考虑旋转
        sceneBounds = entity->sceneBoundingRect();
        actualObjVisualBottomY = sceneBounds.bottom();
    } else {
        // 如果不是 BasePhysicsEntity 或者无法获取 sceneBoundingRect，则回退到原始计算方式
        actualObjVisualBottomY = obj->position().y() + obj->boundingRect().height();
    }

    // 检查物体是否正在向上运动(跳跃中)，如果是则降低容差值
    if (obj->velocity().y() < -10) {
        groundTolerance = 1.0;  // 跳跃时使用更小的容差
    }

    // 检测是否已经穿透地面 (使用旋转后的实际底部)
    if (actualObjVisualBottomY >= terrainHeight) {
        // 物体已经穿透地面，立即校正位置
        qreal currentObjPosX = obj->position().x();
        qreal currentObjPosY = obj->position().y(); // 这是物体自身的锚点Y (通常是左上角)

        if (entity) {
            // 计算需要向上调整的偏移量，使得实际底部与地形高度对齐
            qreal dy_adjust = terrainHeight - actualObjVisualBottomY;
            // 新的锚点Y = 当前锚点Y + 调整量
            obj->setPosition(QPointF(currentObjPosX, currentObjPosY + dy_adjust));
        } else {
            // 回退到原始校正方式 (对于非entity或无sceneBoundingRect的情况)
            obj->setPosition(QPointF(currentObjPosX, terrainHeight - obj->boundingRect().height()));
        }
        handleGroundedState(obj, terrainSlope);
    }
    // 检测是否接近地面但未穿透（使用旋转后的实际底部和容差）
    else if (actualObjVisualBottomY + groundTolerance >= terrainHeight) {
        // 物体接近地面但未穿透

        // 跳跃优化：如果正在向上运动，忽略容差碰撞
        if (obj->velocity().y() < -10) {
            return;
        }

        // 仅当物体向下运动且接近地面时才校正位置
        if (obj->velocity().y() > 0) {
            qreal currentObjPosX = obj->position().x();
            qreal currentObjPosY = obj->position().y();

            if (entity) {
                qreal dy_adjust = terrainHeight - actualObjVisualBottomY;
                obj->setPosition(QPointF(currentObjPosX, currentObjPosY + dy_adjust));
            } else {
                obj->setPosition(QPointF(currentObjPosX, terrainHeight - obj->boundingRect().height()));
            }
            handleGroundedState(obj, terrainSlope);
        }
    }
    else {
        // 物体不在地面附近，设置为非着地状态
        obj->setOnGround(false);
        obj->setSlopeSlideSpeed(0); // 在空中时没有坡面滑动力
    }
}
// 处理着地状态
void GameScene::handleGroundedState(IPhysicsObject* obj, qreal terrainSlope) {
    // 坡面滑动力计算
    qreal slopeSlideForce = 0;
    qreal slopeSlideThreshold = 0.3;
    qreal maxSlideSpeed = 200.0;

    // 根据坡度计算滑动力
    if (terrainSlope > slopeSlideThreshold) {
        slopeSlideForce = terrainSlope * 500.0;
        slopeSlideForce = qMin(slopeSlideForce, maxSlideSpeed);
    }
    else if (terrainSlope < -slopeSlideThreshold) {
        slopeSlideForce = terrainSlope * 200.0;
    }
    obj->setSlopeSlideSpeed(slopeSlideForce);

    // 陡坡飞跃机制
    qreal horizontalSpeed = qAbs(obj->velocity().x());
    qreal verticalSpeed = obj->velocity().y();
    qreal slopeThreshold = 1;
    qreal speedThreshold = 150;

    // 当坡度大且速度快，并且有向上的速度分量时，才认为物体腾空
    if (qAbs(terrainSlope) > slopeThreshold && horizontalSpeed > speedThreshold && verticalSpeed < 0) {
        obj->setOnGround(false);
    } else {
        obj->setVelocity(QPointF(obj->velocity().x(), 0));
        obj->setOnGround(true);
    }
}