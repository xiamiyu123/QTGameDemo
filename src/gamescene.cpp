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
    // 计算物体底部的y坐标（物体顶部坐标加上高度）
    qreal objBottom = obj->position().y() + obj->boundingRect().height();
    // 计算物体中心的x坐标（物体左侧坐标加上宽度的一半）
    qreal objX = obj->position().x() + obj->boundingRect().width() / 2;
    // 获取物体中心x坐标处的地形高度
    qreal terrainHeight = GTerrainGenerator->getTerrainHeight(objX);
    // 获取物体中心x坐标处的地形斜率
    qreal terrainSlope = GTerrainGenerator->getTerrainSlope(objX);

    // 定义地面检测的容差值，允许有小误差
    qreal groundTolerance = 6.0;

    // 检测物体是否接触或接近地面（考虑容差）
    if (objBottom + groundTolerance >= terrainHeight) {
        // 地面接触处理部分
        if (objBottom < terrainHeight) {
            // 如果物体底部在地面以上但很接近，且物体正在下落
            if (obj->velocity().y() > 0) {
                // 调整物体位置，使其恰好站在地形上
                obj->setPosition(QPointF(obj->position().x(), terrainHeight - obj->boundingRect().height()));
            }
        } else {
            // 如果物体底部已经穿过地面，直接调整位置到地面上
            obj->setPosition(QPointF(obj->position().x(), terrainHeight - obj->boundingRect().height()));
        }

        // 计算斜坡效果相关变量
        qreal slopeSlideForce = 0;       // 斜坡滑行力初始为0
        qreal slopeSlideThreshold = 0.3;  // 斜坡效果生效的阈值
        qreal maxSlideSpeed = 200.0;      // 最大滑行速度限制

        // 根据斜坡情况计算滑行力
        if (terrainSlope > slopeSlideThreshold) {
            // 下坡情况：施加向下滑动的力
            slopeSlideForce = terrainSlope * 500.0;
            // 限制最大滑行速度
            slopeSlideForce = qMin(slopeSlideForce, maxSlideSpeed);
        }
        else if (terrainSlope < -slopeSlideThreshold) {
            // 上坡情况：施加阻力（负的滑行力）
            slopeSlideForce = terrainSlope * 200.0;
        }

        // 将计算好的斜坡滑行力应用到物理对象
        obj->setSlopeSlideSpeed(slopeSlideForce);

        // 计算物体是否满足飞跃条件的相关变量
        qreal horizontalSpeed = qAbs(obj->velocity().x());  // 物体水平速度绝对值
        qreal slopeThreshold = 1;     // 陡峭斜坡的阈值
        qreal speedThreshold = 150;    // 触发跳跃的速度阈值

        // 判断是否满足飞跃条件：斜率足够大且速度足够快
        if (qAbs(terrainSlope) > slopeThreshold && horizontalSpeed > speedThreshold) {
            // 标记物体不在地面上
            obj->setOnGround(false);
        } else {
            // 如果不满足飞跃条件，则物体保持在地面上
            obj->setVelocity(QPointF(obj->velocity().x(), 0));
            // 标记物体在地面上
            obj->setOnGround(true);
        }
    } else {
        // 如果物体不在地面上（处于空中）
        obj->setOnGround(false);
        // 清除斜坡滑行速度，因为不在地面上
        obj->setSlopeSlideSpeed(0);
    }
}