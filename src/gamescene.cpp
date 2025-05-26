#include "gamescene.h"

#include <QApplication>
#include <QGraphicsView>
#include <QtSvg>
#include "physical.h"
#include "avalancheupdatethread.h"
#include <QtConcurrent/QtConcurrent>
#include "uimanager.h"
#include "collisionhandler.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "rockentity.h"
#include <QSettings>
#include <QScreen>
GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent), m_uiManager(nullptr), m_collisionHandler(nullptr)
{
    // 设置场景大小（足够大以容纳滚动地形）
    setSceneRect(0, 0, 3000, 30000);

    // 初始化游戏状态变量
    resetGameState();

    // 连接getscore信号到处理函数
    connect(this, &GameScene::getscore, this, &GameScene::onGetScore);

    // 初始化并创建场景关键元素
    createSceneItems();

    // 设置游戏循环定时器
    connect(&GTimer, &QTimer::timeout, this, &GameScene::update);
    GTimer.setInterval(16); // 约60fps

    // 创建UI管理器
    m_uiManager = new UIManager(this, this);
    m_uiManager->initialize();

    // 创建碰撞处理器
    m_collisionHandler = new CollisionHandler(GTerrainGenerator, this);

    // 连接UI事件
    connect(m_uiManager, &UIManager::pauseToggled, this, &GameScene::togglePause);

    // 初始化调试日志器
    DebugLogger::instance()->initialize(this);

    // 创建雪崩
    avalanche = new Avalanche(GTerrainGenerator);
    addItem(avalanche);
    avalanche->setSpeed(150);      // 设置初速度
    avalanche->setAcceleration(5); // 设置加速度
    avalanche->setMaxSpeed(600);   // 设置最大速度

    // 创建并启动雪崩更新线程
    m_avalancheThread = new AvalancheUpdateThread(avalanche, this);
    connect(m_avalancheThread, &AvalancheUpdateThread::updateCompleted,
            this, [this]()
            { avalanche->applyThreadResults(); });
    m_avalancheThread->start();
}

GameScene::~GameScene()
{
    if (m_avalancheThread)
    {
        m_avalancheThread->stop();
        m_avalancheThread->wait();
        delete m_avalancheThread;
        m_avalancheThread = nullptr;
    }

    delete m_uiManager;
    delete m_collisionHandler;
}

void GameScene::initialize()
{
    // 初始化地形
    GTerrainGenerator->initialize();

    // 将玩家放置在适当位置
    initialPlayerPosition();
    // 设置事件过滤器监听视口大小变化
    if (!views().isEmpty())
    {
        views().first()->viewport()->installEventFilter(this);
    }

    // 启动游戏循环
    GElapsedTimer.start();
    GTimer.start();
}

void GameScene::keyPressEvent(QKeyEvent *event)
{ // 把键盘事件传递给玩家
    // 处理暂停和继续
    if (event->key() == Qt::Key_P)
    {
        togglePause();
    }
    // 处理调试框显示/隐藏 - 支持多种~键
    else if (event->key() == Qt::Key_AsciiTilde || // 英文~键
             event->text() == "~" ||               // 文本是~
             event->text() == "～" ||              // 全角～
             event->text() == "`")
    {
        DebugLogger::instance()->toggleVisibility();
    }
    if (event->isAutoRepeat())
    {
        return; // 忽略自动重复事件
    }
    Gplayer->keyPressEvent(event);
    // QGraphicsScene::keyPressEvent(event);
}

void GameScene::keyReleaseEvent(QKeyEvent *event)
{
    // 把键盘事件传递给玩家
    if (event->isAutoRepeat())
    {
        return; // 忽略自动重复事件
    }
    Gplayer->keyReleaseEvent(event);

    // QGraphicsScene::keyReleaseEvent(event);
}

void GameScene::update()
{
    QRectF current = this->sceneRect();
    if (!views().isEmpty())
    {
        QGraphicsView *view = views().first();
        QPointF viewCenterInScene = view->mapToScene(view->viewport()->rect().center());
        if (!current.contains(viewCenterInScene + QPoint(10000, 10000)))
        {
            this->setSceneRect(current.united(QRectF(viewCenterInScene.x() - 1000, viewCenterInScene.y() - 1000,
                                                     20000, 20000)));
            DEBUG_LOG(QString("Scene rect: (%1, %2, %3, %4)")
                          .arg(this->sceneRect().toRect().x())
                          .arg(this->sceneRect().toRect().y())
                          .arg(this->sceneRect().toRect().width())
                          .arg(this->sceneRect().toRect().height()));
        }
    }
    // 时间增量16ms
    qreal deltaTime = 16.0f / 1000.0f;
    // 更新玩家状态
    Gplayer->playerUpdate(GTerrainGenerator);

    // 使用并行处理物理系统中的实体更新
    const QList<IPhysicsObject *> &physicsObjects = PhysicsSystem::instance().getPhysicsObjects();

    // 创建lambda函数以传递deltaTime参数到每个物体的updatePhysics方法
    auto updateFunc = [deltaTime](IPhysicsObject *obj)
    {
        if (obj)
            obj->updatePhysics(deltaTime);
    };

    // 使用QtConcurrent::map并行处理所有物理对象
    QtConcurrent::blockingMap(physicsObjects, updateFunc);

    // 在处理碰撞前清除本帧待删除对象列表
    m_objectsToDeleteThisFrame.clear(); // 处理所有物理对象的碰撞
    for (IPhysicsObject *obj : physicsObjects)
    {
        if (!obj)
        { // 防御性检查
            continue;
        }

        // 检查对象是否已在本帧中被标记为删除
        // 如果是，则跳过对此对象的处理
        bool alreadyMarkedForDeletion = false;
        for (IPhysicsObject *deletedObj : m_objectsToDeleteThisFrame)
        {
            if (obj == deletedObj)
            {
                alreadyMarkedForDeletion = true;
                break;
            }
        }
        if (alreadyMarkedForDeletion)
        {
            continue;
        }

        // 使用碰撞处理器处理物理对象碰撞
        m_collisionHandler->handlePhysicsObjectCollision(obj, m_objectsToDeleteThisFrame);
    }

    // 更新地形生成（基于玩家位置）
    if (GTerrainGenerator)
    {
        GTerrainGenerator->updateTerrain(Gplayer ? Gplayer->x() : 0);
    }

    // 让视图跟随玩家
    centerViewOnPlayer();

    // 雪崩更新改为使用线程
    // 在雪崩更新前添加检查玩家是否被雪崩超越的逻辑
    m_avalancheElapsed += deltaTime;
    if (m_avalancheThread && m_avalancheElapsed >= m_avalancheInterval)
    {
        // 检查玩家是否被雪崩超越
        bool playerSurpassed = avalanche->isPlayerSurpassed(Gplayer->x());

        // 当玩家被超越时，临时将雪崩速度设为0以停止前进
        if (playerSurpassed)
        {
            avalanche->setSpeed(0);
            avalanche->setAcceleration(0);
        }
        else
        {
            // 玩家未被超越时恢复雪崩速度和加速度
            avalanche->setAcceleration(5);
            if (350 + 5 * deltaTime <= 650)
            {
                avalanche->setSpeed(350 + 5 * deltaTime);
            }
            else
            {
                avalanche->setSpeed(650);
            }
        }

        // 请求在线程中更新雪崩
        m_avalancheThread->requestUpdate(m_avalancheElapsed, Gplayer ? Gplayer->x() : 0);
        m_avalancheElapsed = 0;
    }

    // 在所有更新和碰撞处理完成后，实际删除标记的对象
    for (IPhysicsObject *objToDelete : m_objectsToDeleteThisFrame)
    {
        if (objToDelete)
        { // 再次检查，尽管不太可能为null
            delete objToDelete;
        }
    }
    // m_objectsToDeleteThisFrame 会在下一帧 update 开始时被清空
    // 检查玩家是否触碰到雪崩
    QRectF playerRect = Gplayer->sceneBoundingRect();
    QPainterPath avalanchePath = avalanche->path();
    QPainterPath playerPath;

    // 创建一个比玩家实际碰撞箱小的区域（内缩10像素）
    QRectF toleranceRect = playerRect.adjusted(10, 10, -10, -10);
    playerPath.addRect(toleranceRect);

    // 如果玩家碰到雪崩，游戏结束
    if (avalanchePath.intersects(playerPath) &&
        avalanche->isPlayerSignificantlyInside(Gplayer->sceneBoundingRect()))
    {
        GTimer.stop();
        showGameOverDialog();
        return;
    }

    // 更新警告图标
    qreal dist = avalanche->distanceToPlayer(Gplayer->x());
    if (dist < 2500)
    {
        m_uiManager->showWarningIndicator(true, dist);
    }
    else
    {
        m_uiManager->showWarningIndicator(false, dist);
    }
}

// 仅用于初始化时放置玩家
void GameScene::initialPlayerPosition()
{

    qreal terrainHeight = GTerrainGenerator->getTerrainHeight(1200);
    Gplayer->setY(terrainHeight - Gplayer->rect().height());
    Gplayer->setX(1200); // 玩家方块偏左一点以更符合滑雪大冒险
    Gplayer->setOnGround(true);
}

void GameScene::centerViewOnPlayer()
{
    if (views().isEmpty())
    {
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
    if (GState == Running)
    {
        GState = Paused;
        GTimer.stop();
        m_uiManager->showPauseOverlay(true, score);
    }
    else
    {
        GState = Running;
        m_uiManager->showPauseOverlay(false);
        GTimer.start();
    }
}

// 在GameScene类中添加事件过滤器方法
bool GameScene::eventFilter(QObject *watched, QEvent *event)
{
    if (views().isEmpty())
        return false;
    // 监听视口的调整大小事件
    if (watched == views().first()->viewport() && event->type() == QEvent::Resize)
    {
        m_uiManager->updateUI();

        // 更新调试日志框位置
        DebugLogger::instance()->updatePosition();
    }
    return QGraphicsScene::eventFilter(watched, event);
}

// 新增：处理得分的槽函数
void GameScene::onGetScore(int points)
{
    // 应用分数奖励倍数
    int adjustedPoints = static_cast<int>(points * award_score);
    // 增加玩家得分
    score += adjustedPoints;

    DEBUG_LOG(QString("玩家得分: %1 (奖励倍数: %2)").arg(score).arg(award_score));
}

// 显示游戏结束对话框
void GameScene::showGameOverDialog()
{
    qreal secs = GElapsedTimer.elapsed() / 1000.0;

    // 使用UIManager显示游戏结束对话框
    m_uiManager->showGameOverDialog(secs, [this]()
                                    {
            // 重试逻辑
            // 停止定时器和线程
            GTimer.stop();
            if (m_avalancheThread) {
                m_avalancheThread->stop();
                m_avalancheThread->wait();
                delete m_avalancheThread;
                m_avalancheThread = nullptr;
            }
            // 重置并重建
            resetGameState();
            clear();
            createSceneItems();
            
            // 更新碰撞处理器中的地形生成器引用
            if (m_collisionHandler) {
                m_collisionHandler->updateTerrainGenerator(GTerrainGenerator);
            }
            
            initialize(); }, [this]()
                                    {
            // 退出逻辑
            GTimer.stop();
            if (m_avalancheThread) {
                m_avalancheThread->stop();
                m_avalancheThread->wait();
                delete m_avalancheThread;
                m_avalancheThread = nullptr;
            }
            qApp->quit(); });
}

// 重置游戏状态
void GameScene::resetGameState()
{
    // 设置游戏状态
    GState = Running;

    // 初始化得分和奖励倍数
    score = 0;
    award_speed = 1.0;
    award_score = 1.0;

    // 重置更新计时器和对象列表
    m_avalancheElapsed = 0;
    m_objectsToDeleteThisFrame.clear();

    // 停止游戏定时器
    GTimer.stop();
    // GElapsedTimer 将在 initialize 中重启
}

// 创建场景关键元素
void GameScene::createSceneItems()
{
    // 地形生成器
    GTerrainGenerator = new TerrainGenerator(this, this);

    // 玩家
    Gplayer = new Player();
    addItem(Gplayer);
    PhysicsSystem::instance().registerObject(Gplayer);

    // 雪崩及线程
    avalanche = new Avalanche(GTerrainGenerator);
    addItem(avalanche);
    avalanche->setSpeed(150);
    avalanche->setAcceleration(5);
    avalanche->setMaxSpeed(600);
    m_avalancheThread = new AvalancheUpdateThread(avalanche, this);
    connect(m_avalancheThread, &AvalancheUpdateThread::updateCompleted,
            this, [this]()
            { avalanche->applyThreadResults(); });
    m_avalancheThread->start();
}