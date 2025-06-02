#include "gamescene.h"

#include <QApplication>
#include <QGraphicsView>
#include <QtSvg>
#include "physical.h"
#include "avalancheupdatethread.h"
#include <QtConcurrent/QtConcurrent>
#include "uimanager.h"
#include "collisionhandler.h"
#include "npcentity.h"

#include <QDialog>
#include "rockentity.h"

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent),
      m_uiManager(nullptr),
      m_collisionHandler(nullptr),
      m_npcSpawnTimer(nullptr),
      m_npcSpawnDistance(800.0),
      m_maxNPCCount(10),
      m_npcCleanupDistance(500.0)
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
    m_collisionHandler = new CollisionHandler(GTerrainGenerator, this);    // 连接UI事件
    connect(m_uiManager, &UIManager::pauseToggled, this, &GameScene::togglePause);
      // 连接玩家NPC拾取冷却进度条信号
    connect(Gplayer, &Player::npcPickupCooldownChanged,
            m_uiManager, &UIManager::showNPCPickupCooldown);

    // 连接玩家坠落恢复进度条信号
    connect(Gplayer, &Player::fallRecoveryChanged,
            m_uiManager, &UIManager::showFallRecovery);

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

    // 初始化得分相关
    m_lastScoredPositionX = 1200;
    m_scoreDistance = 100;  // 每100像素得分一次

    // // 连接玩家摔倒信号（测试用）
    // connect(Gplayer, &Player::playerFallen, m_uiManager, &UIManager::showScorePopup);
    //
    // // 同时连接到加分系统
    // connect(Gplayer, &Player::playerFallen, this, [this](int points, const QString&) {
    //     emit getscore(points);
    // });
    m_avalancheThread->start();    // 运行测试构造代码
    test();

    // 初始化NPC系统
    initializeNPCSystem();
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

    // 清理NPC系统
    if (m_npcSpawnTimer) {
        m_npcSpawnTimer->stop();
        delete m_npcSpawnTimer;
    }

    // 清理所有NPC（unique_ptr会自动删除）
    for (auto& npc : m_activeNPCs) {
        if (npc) {
            removeItem(npc.get());
        }
    }
    m_activeNPCs.clear();

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
    Gplayer->playerUpdate(GTerrainGenerator);    // 单线程处理物理系统中的实体更新（Qt对象非线程安全）
    const QList<IPhysicsObject *> &physicsObjects = PhysicsSystem::instance().getPhysicsObjects();

    // 顺序更新所有物理对象
    for (IPhysicsObject *obj : physicsObjects)
    {
        if (obj)
            obj->updatePhysics(deltaTime);
    }

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
            avalanche->setAcceleration(avalanche->getInitialAcceleration());
            if (avalanche->getInitialSpeed() + avalanche->getInitialAcceleration() * deltaTime <= avalanche->getMaxSpeed())
            {
                avalanche->setSpeed(avalanche->getInitialSpeed() + avalanche->getInitialAcceleration() * deltaTime);
            }
            else
            {
                avalanche->setSpeed(avalanche->getMaxSpeed());
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

    // === NPC系统更新 ===
    updateAllNPCs(deltaTime);

    checkPlayerProgressScore();
}

// 仅用于初始化时放置玩家
void GameScene::initialPlayerPosition()
{
    qreal terrainHeight = GTerrainGenerator->getTerrainHeight(1200);

    // 使用局部坐标系统设置位置，与物理系统保持一致
    Gplayer->setX(1200);
    Gplayer->setY(terrainHeight - Gplayer->rect().height());
    Gplayer->setOnGround(true);

    DEBUG_LOG(QString("Player initial position set to Local: (%1, %2)")
              .arg(Gplayer->x(), 0, 'f', 1)
              .arg(Gplayer->y(), 0, 'f', 1));
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
    // 安全检查：确保UI管理器和游戏状态都准备就绪
    if (!m_uiManager || !Gplayer || !GTerrainGenerator) {
        return; // 游戏正在重建中，忽略暂停请求
    }
    
    if (GState == GameState::Running)
    {
        GState = GameState::Paused;
        GTimer.stop();
        m_uiManager->showPauseOverlay(true, score);
    }
    else if (GState == GameState::Paused)
    {
        GState = GameState::Running;
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

    if (m_uiManager) {
        m_uiManager->setScore(score);
    }
}

// 显示游戏结束对话框
void GameScene::showGameOverDialog()
{
    GState = GameOver;
    // 使用UIManager显示游戏结束对话框
    m_uiManager->showGameOverDialog(score, [this]()
                                    {
            // 重试逻辑
            // 临时断开UI信号连接，防止在重建过程中触发暂停
            disconnect(m_uiManager, &UIManager::pauseToggled, this, &GameScene::togglePause);
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
            clearGameObjects();
            createSceneItems();

            // 更新碰撞处理器中的地形生成器引用
            if (m_collisionHandler) {
                m_collisionHandler->updateTerrainGenerator(GTerrainGenerator);
            }

        // // 重新连接玩家摔倒信号（测试用）
        // connect(Gplayer, &Player::playerFallen, m_uiManager, &UIManager::showScorePopup);
        // connect(Gplayer, &Player::playerFallen, this, [this](int points, const QString&) {
        //     emit getscore(points);
        //      });

        // 重置UI状态
        if (m_uiManager) {
            m_uiManager->resetUI();
        }

            initialize();

            // 重新连接UI信号
        connect(m_uiManager, &UIManager::pauseToggled, this, &GameScene::togglePause);
            }, [this]()
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
    GState = GameState::Running;

    // 初始化得分和奖励倍数
    score = 0;
    award_speed = 1.0;
    award_score = 1.0;

    // 重置计分位置
    m_lastScoredPositionX = 1200;


    // 重置更新计时器和对象列表
    m_avalancheElapsed = 0;
    m_objectsToDeleteThisFrame.clear();

    // 停止游戏定时器
    GTimer.stop();

    // 手动删除游戏对象，保留UI元素
    // 首次启动时不需要清理，只有在重试或退出时才需要
    static bool first = true;
    if (!first) {
        clearGameObjects();
    }
    first = false; // 确保只在第一次初始化时不清理


    // GElapsedTimer 将在 initialize 中重启
}

// 添加新的方法来清理游戏对象
void GameScene::clearGameObjects()
{
    // 删除玩家
    if (Gplayer) {
        removeItem(Gplayer);
        PhysicsSystem::instance().unregisterObject(Gplayer);
        delete Gplayer;
        Gplayer = nullptr;
    }

    // 删除雪崩
    if (avalanche) {
        removeItem(avalanche);
        delete avalanche;
        avalanche = nullptr;
    }

    // 清理地形生成器
    if (GTerrainGenerator) {
        GTerrainGenerator->clearAllResources();
        delete GTerrainGenerator;
        GTerrainGenerator = nullptr;
    }
      // 清理物理系统中剩余的对象
    const QList<IPhysicsObject*>& physicsObjects = PhysicsSystem::instance().getPhysicsObjects();
    QList<IPhysicsObject*> objectsToDelete = physicsObjects;

    for (IPhysicsObject* obj : objectsToDelete) {
        if (obj) {
            BasePhysicsEntity* entity = dynamic_cast<BasePhysicsEntity*>(obj);
            if (entity) {
                removeItem(entity);
            }
            PhysicsSystem::instance().unregisterObject(obj);
            delete obj;
        }
    }    // 强制清理所有剩余的图形项目，排除UI元素
    QList<QGraphicsItem*> allItems = items();
    for (QGraphicsItem* item : allItems) {
        // 保留UIManager管理的所有UI元素
        if (m_uiManager && m_uiManager->isUIManagerObject(item)) {
            continue;
        }
        
        removeItem(item);
        delete item;
    }
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

    // 连接玩家空翻成功信号
    connect(Gplayer, &Player::backFlipSuccess, this, &GameScene::onBackFlipSuccess);

    // 连接玩家摔倒重置倍率信号
    connect(Gplayer, &Player::resetAwardMultipliers, this, &GameScene::resetAwardMultipliers);


    // 连接玩家摔倒信号（测试用）
    // connect(Gplayer, &Player::playerFallen, m_uiManager, &UIManager::showScorePopup);
    // connect(Gplayer, &Player::playerFallen, this, [this](int points, const QString&) {
    //     emit getscore(points);
    // });

}

void GameScene::checkPlayerProgressScore() {
    if (Gplayer) {
        qreal currentX = Gplayer->pos().x();

        // 只有当玩家向右移动时才计分
        if (currentX > m_lastScoredPositionX + m_scoreDistance) {
            // 计算玩家移动了多少个得分距离
            int scoreUnits = static_cast<int>((currentX - m_lastScoredPositionX) / m_scoreDistance);
            int points = scoreUnits * 10;  // 每单位距离得10分

            // 更新最后得分位置
            m_lastScoredPositionX += scoreUnits * m_scoreDistance;

            // 发射得分信号
            emit getscore(points * award_score); // 应用分数奖励倍数

        }
    }
}

void GameScene::onBackFlipSuccess(int points, const QString& message) {
    // 应用分数奖励倍数
    int adjustedPoints = static_cast<int>(points * award_score);
    score += adjustedPoints;

    // 增加奖励倍数（限制最大值以避免游戏过于简单）
    award_score = qMin(award_score * 1.2, 2.5);  // 增加20%的得分倍率，最大2.5倍
    award_speed = qMin(award_speed * 1.2, 1.2);  // 增加20%的速度倍率，最大1.5 倍

    // 更新UI显示
    if (m_uiManager) {
        m_uiManager->setScore(score);
        m_uiManager->showScorePopup(adjustedPoints, message);
    }

    // 启动加速效果
    Gplayer->startFlipBoost();

    DEBUG_LOG(QString("空翻奖励: %1分 (得分倍数: %2, 速度倍数: %3)")
        .arg(adjustedPoints).arg(award_score).arg(award_speed));

    // 将速度倍数应用到玩家
    if (Gplayer->isFlipBoosting()) {
        Gplayer->setMoveSpeed(Gplayer->getInitialMoveSpeed() * award_speed);  // 基础速度 * 速度倍率
    }
}

// === NPC管理系统实现 ===

void GameScene::initializeNPCSystem()
{
    // 暂时禁用随机NPC生成系统
    // 只保留地形生成时创建的企鹅NPC

    // 弃用的创建NPC生成定时器但不启动
    // m_npcSpawnTimer = new QTimer(this);
    // connect(m_npcSpawnTimer, &QTimer::timeout, this, &GameScene::spawnNPC);

    // 不启动定时器
    // m_npcSpawnTimer->setInterval(QRandomGenerator::global()->bounded(3000, 5000));
    // m_npcSpawnTimer->start();

    DEBUG_LOG("NPC系统已初始化");
}

void GameScene::spawnNPC()
{
    if (!Gplayer) {
        return;
    }

    // 检查是否超过最大NPC数量
    if (m_activeNPCs.size() >= m_maxNPCCount) {
        return;
    }

    // 清理过期的NPC
    cleanupNPCs();

    // 获取生成位置
    QPointF spawnPos = getNPCSpawnPosition();

    // 随机选择NPC类型
    spawnRandomNPC(spawnPos);

    // 重置定时器间隔
    m_npcSpawnTimer->setInterval(QRandomGenerator::global()->bounded(3000, 5000));
}

void GameScene::spawnRandomNPC(const QPointF& position)
{

}

void GameScene::updateAllNPCs(float deltaTime)
{
    if (!Gplayer || !GTerrainGenerator || !avalanche) {
        return;
    }
      qreal playerX = Gplayer->x();
    qreal viewWidth = 1200; // 估计的视图宽度
    qreal activationDistance = viewWidth * 1.5; // 增加NPC激活距离，确保更早激活
    qreal avalancheFrontX = avalanche->getFrontX();

    // 遍历地形生成器中的所有NPC
    auto& npcs = GTerrainGenerator->m_npcs;
    auto it = npcs.begin();

    while (it != npcs.end()) {
        NPCEntity* npc = *it;

        if (!npc) {
            it = npcs.erase(it);
            continue;
        }

        qreal npcX = npc->x();
        bool shouldRemove = false;

        // 检查是否被雪崩追上
        if (npcX <= avalancheFrontX) {
            DEBUG_LOG(QString("NPC被雪崩追上，位置: %1, 雪崩前沿: %2").arg(npcX).arg(avalancheFrontX));
            shouldRemove = true;
        }
        // 检查是否到了还没有生成的地块
        else {
            int currentChunk = static_cast<int>(std::floor(npcX / 3600)); // CHUNK_WIDTH = 3600
            int playerChunk = static_cast<int>(std::floor(playerX / 3600));

            // 如果NPC在玩家前方超过2个地形块，说明该地形块可能还没生成
            if (currentChunk > playerChunk + 2) {
                DEBUG_LOG(QString("NPC到达未生成地块，NPC地块: %1, 玩家地块: %2").arg(currentChunk).arg(playerChunk));
                shouldRemove = true;
            }
        }

        if (shouldRemove) {
            // 从场景移除
            removeItem(npc);

            // 从物理系统注销
            PhysicsSystem::instance().unregisterObject(npc);

            // 删除NPC对象
            delete npc;

            // 从列表移除
            it = npcs.erase(it);

            DEBUG_LOG("回收了一个NPC");
        } else {
            // 检查是否需要激活NPC
            if (!npc->isActive()) {
                // NPC进入玩家前方视野范围时激活
                if (npcX >= playerX - activationDistance && npcX <= playerX + activationDistance) {
                    npc->setActive(true);
                    DEBUG_LOG(QString("激活NPC，位置: %1, 玩家位置: %2").arg(npcX).arg(playerX));
                }
            }

            ++it;
        }
    }
}

void GameScene::cleanupNPCs()
{
    auto it = m_activeNPCs.begin();
    while (it != m_activeNPCs.end()) {
        NPCEntity* npc = it->get();

        // 检查是否应该销毁
        if (npc && npc->shouldDestroy()) {
            // 从场景移除
            removeItem(npc);

            // 从物理系统注销
            PhysicsSystem::instance().unregisterObject(npc);

            // 从列表移除（unique_ptr会自动删除对象）
            it = m_activeNPCs.erase(it);

            DEBUG_LOG("清理了一个NPC");
        } else {
            ++it;
        }
    }
}

void GameScene::removeOffscreenNPCs()
{
    if (!Gplayer) {
        return;
    }

    qreal playerX = Gplayer->x();

    auto it = m_activeNPCs.begin();
    while (it != m_activeNPCs.end()) {
        NPCEntity* npc = it->get();

        // 检查NPC是否离玩家太远（在左边或右边）
        if (npc && (npc->x() < playerX - m_npcCleanupDistance)) {
            // NPC在玩家左边太远，移除
            removeItem(npc);
            PhysicsSystem::instance().unregisterObject(npc);
            it = m_activeNPCs.erase(it);

            DEBUG_LOG("移除了离屏幕太远的NPC");
        } else {
            ++it;
        }
    }
}

QPointF GameScene::getNPCSpawnPosition()
{
    if (!Gplayer) {
        return QPointF(0, 0);
    }

    qreal playerX = Gplayer->x();
    qreal spawnX = playerX + m_npcSpawnDistance;

    // 获取地形高度
    qreal groundHeight = 0;
    if (GTerrainGenerator) {
        groundHeight = GTerrainGenerator->getTerrainHeight(spawnX);
    }

    // 随机决定是在地面还是空中生成
    bool spawnInAir = QRandomGenerator::global()->bounded(3) == 0; // 1/3概率在空中

    qreal spawnY;
    if (spawnInAir) {
        // 在空中生成（地面上方100-300像素）
        spawnY = groundHeight - QRandomGenerator::global()->bounded(100, 300);
    } else {
        // 在地面生成
        spawnY = groundHeight - 25; // NPC高度的一半
    }

    return QPointF(spawnX, spawnY);
}

void GameScene::resetAwardMultipliers()
{
    // 重置得分倍率
    award_score = 1.0;

    DEBUG_LOG(QString("摔倒重置: 得分倍数=%1")
        .arg(award_score));


}