#include "gamescene.h"
#include <QGraphicsView>
#include <QtSvg>
#include "physical.h"
static qreal lastSlope = 0;
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
    pauseButton->setIcon(QIcon(":/resource/images/icons/pause.svg"));
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
    // 更新玩家状态
    Gplayer->playerUpdate(GTerrainGenerator);

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

    // 获取玩家的场景中心点
    QPointF playerCenter = Gplayer->sceneBoundingRect().center();

    playerCenter.rx() += 200; // 让玩家在视图中偏左


    view->centerOn(playerCenter);
}

void GameScene::togglePause()
{
    if (GState == Running) {
        GState = Paused;
        GTimer.stop();
        qreal secs = GElapsedTimer.elapsed() / 1000.0;
        GPauseText->setPlainText(QString("游戏已暂停，已进行" + QString::number(secs, 'f', 2) + "秒"));
        //更改暂停按钮图标
        pauseButton->setIcon(QIcon(":/resource/images/icons/play.svg"));
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
        pauseButton->setIcon(QIcon(":/resource/images/icons/pause.svg"));
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
    // 计算物体中心的x坐标
    qreal objX = obj->position().x() + obj->boundingRect().width() / 2;
    // 获取地形高度
    qreal terrainHeight = GTerrainGenerator->getTerrainHeight(objX) + 2; //模拟陷入雪中
    // 获取地形斜率
    qreal terrainSlope = GTerrainGenerator->getTerrainSlope(objX);

    // 获取前方地形斜率，用于预测
    qreal forwardX = objX + 30; // 向前看30像素
    qreal forwardSlope = GTerrainGenerator->getTerrainSlope(forwardX);

    BasePhysicsEntity* entity = dynamic_cast<BasePhysicsEntity*>(obj);

    // 设置物体旋转角度以匹配地形斜率
    if (entity && obj->isOnGround()) {
        qreal angle = qAtan(terrainSlope) * 180.0 / M_PI;
        entity->setRotation(angle);
    }

    // 获取物体旋转后的实际底部Y坐标
    qreal actualObjVisualBottomY;
    QRectF sceneBounds;

    if (entity) {
        sceneBounds = entity->sceneBoundingRect();
        actualObjVisualBottomY = sceneBounds.bottom();
    } else {
        actualObjVisualBottomY = obj->position().y() + obj->boundingRect().height();
    }

    // 动态地面容差计算 - 根据速度和斜率调整
    qreal horizontalSpeed = qAbs(obj->velocity().x());
    qreal groundTolerance = 6.0; // 基础容差

    // 高速下坡时增加容差
    if (horizontalSpeed > 100 && terrainSlope < -0.2) {
        // 速度越快，下坡越陡，容差越大
        groundTolerance = qMin(20.0, 6.0 + horizontalSpeed * 0.05);
    }

    // 下坡预测校正 - 如果前方斜率更陡，增加容差
    if (forwardSlope < terrainSlope && forwardSlope < -0.3 && horizontalSpeed > 150) {
        groundTolerance += 5.0;
    }

    // 跳跃时减小容差
    if (obj->velocity().y() < -10) {
        groundTolerance = 1.0;
    }

    // 检测是否已经穿透地面
    if (actualObjVisualBottomY >= terrainHeight) {
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
    // 高速下坡额外逻辑 - 更主动地贴合地面
    else if (horizontalSpeed > 150 && terrainSlope < -0.4 && obj->isOnGround() &&
             actualObjVisualBottomY + groundTolerance*2 >= terrainHeight) {
        // 高速下陡坡时，主动向下移动以保持贴合
        qreal currentObjPosX = obj->position().x();
        qreal currentObjPosY = obj->position().y();

        if (entity) {
            // 更主动的位置调整
            qreal dy_adjust = (terrainHeight - actualObjVisualBottomY) * 0.5; // 部分调整
            obj->setPosition(QPointF(currentObjPosX, currentObjPosY + dy_adjust));
        }

        // 保持接地状态
        obj->setOnGround(true);
    }
    // 接近地面检测
    else if (actualObjVisualBottomY + groundTolerance >= terrainHeight) {
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
        obj->setSlopeSlideSpeed(0);
    }
}
// 处理着地状态
void GameScene::handleGroundedState(IPhysicsObject* obj, qreal terrainSlope) {
    // 坡面滑动力计算
    qreal slopeSlideForce = 0;
    qreal slopeSlideThreshold = 0.3;
    qreal maxSlideSpeed = 200.0;

    if (terrainSlope > slopeSlideThreshold) {
        slopeSlideForce = terrainSlope * 500.0;
        slopeSlideForce = qMin(slopeSlideForce, maxSlideSpeed);
    } else if (terrainSlope < -slopeSlideThreshold) {
        slopeSlideForce = terrainSlope * 200.0;
    }
    obj->setSlopeSlideSpeed(slopeSlideForce);


    // --- 飞跃判定优化 ---
    bool shouldTakeoff = false;
    qreal horizontalSpeed = qAbs(obj->velocity().x());
    qreal verticalSpeed = obj->velocity().y();

    // 只在“刚刚落地”时允许飞跃判定，防止连续弹跳
    const qreal landingVerticalSpeedThreshold = 20.0;
    bool isLanding = qAbs(verticalSpeed) < landingVerticalSpeedThreshold;

    // 坡顶检测
    if (lastSlope > 0.5 && terrainSlope < -0.2 && isLanding) {
        shouldTakeoff = true;
    }
    // 速度与坡度综合判定
    if (!shouldTakeoff && qAbs(terrainSlope) > 0.4 && horizontalSpeed > 80 && isLanding) {
        shouldTakeoff = true;
    }
    lastSlope = terrainSlope;

    if (shouldTakeoff) {
        obj->setOnGround(false);
        // 不归零竖直速度
    } else {
        // 只要碰到地面就直接落地
        obj->setVelocity(QPointF(obj->velocity().x(), 0));
        obj->setOnGround(true);
    }
}