#include "gamescene.h"

#include <QApplication>
#include <QGraphicsView>
#include <QtSvg>
#include "physical.h"
#include "avalancheupdatethread.h"
#include <QtConcurrent/QtConcurrent>

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "rockentity.h"
#include <QSettings>
#include <QScreen>
static qreal lastSlope = 0;
GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent)
{
    // 设置场景大小（足够大以容纳滚动地形）
    setSceneRect(0, 0, 2000000, 2000000);

    // 初始化游戏状态变量
    resetGameState();

    // 连接getscore信号到处理函数
    connect(this, &GameScene::getscore, this, &GameScene::onGetScore);

    // 初始化并创建场景关键元素
    createSceneItems();

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

    warningButton = new QPushButton();
    warningButton->setIcon(QIcon(":/resource/images/icons/warning.png"));
    warningButton->setFocusPolicy(Qt::NoFocus);
    warningButton->setIconSize(QSize(50, 50));
    warningButton->setStyleSheet("QPushButton { background: transparent; border: none; }");
    warningButton->setAttribute(Qt::WA_TranslucentBackground);
    warningButton->hide();

    // 创建雪崩
    avalanche = new Avalanche(GTerrainGenerator);
    addItem(avalanche);
    avalanche->setSpeed(150);        // 设置初速度
    avalanche->setAcceleration(5);  // 设置加速度
    avalanche->setMaxSpeed(600);    // 设置最大速度

    // 创建并启动雪崩更新线程
    m_avalancheThread = new AvalancheUpdateThread(avalanche, this);
    connect(m_avalancheThread, &AvalancheUpdateThread::updateCompleted,
            this, [this]() {
                avalanche->applyThreadResults();
            });
    m_avalancheThread->start();
}

GameScene::~GameScene()
{
    if (m_avalancheThread) {
        m_avalancheThread->stop();
        m_avalancheThread->wait();
    }
}

void GameScene::initialize()
{
    // 初始化地形
    GTerrainGenerator->initialize();

    // 将玩家放置在适当位置
    initialPlayerPosition();
    
    // 设置事件过滤器监听视口大小变化
    if (!views().isEmpty()) {
        views().first()->viewport()->installEventFilter(this);
    }

    // 初始设置UI
    updateUI();

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

    // 使用并行处理物理系统中的实体更新
    const QList<IPhysicsObject*>& physicsObjects = PhysicsSystem::instance().getPhysicsObjects();

    // 创建lambda函数以传递deltaTime参数到每个物体的updatePhysics方法
    auto updateFunc = [deltaTime](IPhysicsObject* obj) {
        if (obj) obj->updatePhysics(deltaTime);
    };

    // 使用QtConcurrent::map并行处理所有物理对象
    QtConcurrent::blockingMap(physicsObjects, updateFunc);

    // 在处理碰撞前清除本帧待删除对象列表
    m_objectsToDeleteThisFrame.clear();

    // 处理所有物理对象的碰撞
    for (IPhysicsObject* obj : physicsObjects) {
        if (!obj) { // 防御性检查
            continue;
        }

        // 检查对象是否已在本帧中被标记为删除
        // 如果是，则跳过对此对象的处理
        bool alreadyMarkedForDeletion = false;
        for (IPhysicsObject* deletedObj : m_objectsToDeleteThisFrame) {
            if (obj == deletedObj) {
                alreadyMarkedForDeletion = true;
                break;
            }
        }
        if (alreadyMarkedForDeletion) {
            continue;
        }

        handlePhysicsObjectCollision(obj);
    }

    // 更新地形生成（基于玩家位置）
    if (GTerrainGenerator) {
        GTerrainGenerator->updateTerrain(Gplayer ? Gplayer->x() : 0);
    }

    // 让视图跟随玩家
    centerViewOnPlayer();

    // 雪崩更新改为使用线程
    m_avalancheElapsed += deltaTime;
    if (m_avalancheThread && m_avalancheElapsed >= m_avalancheInterval) {
        // 请求在线程中更新雪崩
        m_avalancheThread->requestUpdate(m_avalancheElapsed, Gplayer ? Gplayer->x() : 0);
        m_avalancheElapsed = 0;
    }

    // 在所有更新和碰撞处理完成后，实际删除标记的对象
    for (IPhysicsObject* objToDelete : m_objectsToDeleteThisFrame) {
        if (objToDelete) { // 再次检查，尽管不太可能为null
            delete objToDelete;
        }
    }
    // m_objectsToDeleteThisFrame 会在下一帧 update 开始时被清空

    // 检查玩家是否被雪崩追上
    if (avalanche->isPlayerCaught(Gplayer->x())) {
        GTimer.stop();
        showGameOverDialog();
        return;
    }

    qreal dist = avalanche->distanceToPlayer(Gplayer->x());
    if (dist < 2500) {
        warningButton->show();
        int size;
        if (dist < 800) {
            size = 80;
        } else if (dist < 1200 && dist >= 800) {
            size = 50 + int((1200 - dist) / 400.0 * 30);
        } else {
            size = 50;
        }
        warningButton->setIconSize(QSize(size, size));
        warningButton->setGeometry(10, 10, size, size);
    } else {
        warningButton->hide();
    }
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

        // 创建渐变效果 - 从深蓝色边缘到浅蓝色中心的径向渐变
        QGraphicsView *view = views().first();
        QRectF viewRect = view->mapToScene(view->viewport()->rect()).boundingRect();
        
        // 设置遮罩区域覆盖整个可视区域
        GPauseOverlay->setRect(viewRect);
        
        // 创建从边缘深蓝到中心浅蓝的径向渐变
        QRadialGradient gradient(viewRect.center(), qMax(viewRect.width(), viewRect.height()) / 2);
        gradient.setColorAt(0.0, QColor(100, 180, 255, 180));   // 中心浅蓝色，半透明
        gradient.setColorAt(1.0, QColor(10, 50, 120, 230));     // 边缘深蓝色，更不透明
        
        GPauseOverlay->setBrush(gradient);
        GPauseOverlay->setPen(Qt::NoPen);  // 无边框
        GPauseOverlay->show();

        // 更新暂停文字内容，现在显示分数而不是时间
        GPauseText->setPlainText(QString("游戏已暂停\n\n当前分数：" + QString::number(score) + " 分"));

        // 更改暂停按钮图标
        pauseButton->setIcon(QIcon(":/resource/images/icons/play.svg"));

        // 显示暂停文字（位置更新由updateUI负责）
        GPauseText->show();

        // 立即更新一次UI以定位暂停文字
        updateUI();
    } else {
        GState = Running;
        GPauseText->hide();
        GPauseOverlay->hide();
        GTimer.start();
        //更改暂停按钮图标
        pauseButton->setIcon(QIcon(":/resource/images/icons/pause.svg"));
    }
}


void GameScene::updateUI()
{   if (views().isEmpty()) return;
    QGraphicsView *view = views().first();
    // 确保按钮有父对象
    pauseButton->setParent(view->viewport());
    // 固定暂停按钮在右上角，10px 边距
    QRect vp = view->viewport()->rect();
    pauseButton->setGeometry(vp.width() - 50 - 10, 10, 50, 50);
    pauseButton->show();

    // 如果当前状态是暂停，也需要更新暂停文字的位置
    if (GState == Paused && GPauseText->isVisible()) {
        QRectF viewSceneRect = view->mapToScene(view->viewport()->rect()).boundingRect();
        QRectF textRect = GPauseText->boundingRect();
        // 文字居中到视口
        GPauseText->setPos(
            viewSceneRect.center().x() - textRect.width() / 2,
            viewSceneRect.center().y() - textRect.height() / 2
        );
    }
    // 更新警告按钮位置
    warningButton->setParent(view->viewport());
    warningButton->setGeometry(10, 10, warningButton->iconSize().width(), warningButton->iconSize().height());
}

// 在GameScene类中添加事件过滤器方法
bool GameScene::eventFilter(QObject *watched, QEvent *event)
{
    if (views().isEmpty()) return false;
    // 监听视口的调整大小事件
    if (watched == views().first()->viewport() && event->type() == QEvent::Resize) {
        updateUI();
    }
    return QGraphicsScene::eventFilter(watched, event);
}


// 处理物理对象与地形的碰撞
void GameScene::handlePhysicsObjectCollision(IPhysicsObject* obj) {
    // 获取物体信息
    QRectF objRect = obj->boundingRect(); // 此处是崩溃点 (gamescene.cpp:236)
    QPointF objPos = obj->position();
    qreal objCenterX = objPos.x() + objRect.width() / 2;

    // 多点采样获取地形信息 - 更好地适应不规则地形
    qreal centerTerrain = GTerrainGenerator->getTerrainHeight(objCenterX);
    qreal leftTerrain = GTerrainGenerator->getTerrainHeight(objCenterX - objRect.width() * 0.4);
    qreal rightTerrain = GTerrainGenerator->getTerrainHeight(objCenterX + objRect.width() * 0.4);
    qreal terrainHeight = qMin(qMin(leftTerrain, centerTerrain), rightTerrain);

    // 获取当前和预测的地形斜率
    qreal terrainSlope = GTerrainGenerator->getTerrainSlope(objCenterX);
    qreal backSlope = GTerrainGenerator->getTerrainSlope(objCenterX - objRect.width() * 0.6);
    qreal forwardSlope = GTerrainGenerator->getTerrainSlope(objCenterX + objRect.width() * 0.6);

    // 获取物体速度
    QPointF velocity = obj->velocity();
    qreal horizontalSpeed = qAbs(velocity.x());
    qreal verticalSpeed = velocity.y();

    // 获取实体对象和实际底部位置
    BasePhysicsEntity* entity = dynamic_cast<BasePhysicsEntity*>(obj);

    qreal actualObjBottom;
    if (entity) {
        actualObjBottom = entity->sceneBoundingRect().bottom() - 5;
    }
    else {
        actualObjBottom = objPos.y() + objRect.height();
    }

    // 动态地面检测容差 - 核心改进
    qreal groundTolerance = calculateGroundTolerance(horizontalSpeed, terrainSlope, forwardSlope, verticalSpeed);

    // 添加玩家对象的检测
    Player* player = dynamic_cast<Player*>(obj); // 注意：这里的 player 变量名可能会与函数参数 obj 混淆，但它是局部变量

    bool wasOnGround = obj->isOnGround();

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
                player->checkLanding(terrainAngle);
            }
            obj->setVelocity(QPointF(velocity.x(), 0));
             qDebug() << "落地: 地形高度 =" << terrainHeight << "角色底部 =" << actualObjBottom;
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
                qDebug() << "靠近地面落地: 地形高度 =" << terrainHeight << "角色底部 =" << actualObjBottom;
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
    }

    // 玩家与石头碰撞检测 - 仅当当前 obj 是玩家时执行
    if (player) { // player 是 dynamic_cast<Player*>(obj) 的结果
        for (int i = GTerrainGenerator->m_rocks.size() - 1; i >= 0; --i) {
            RockEntity* rock = GTerrainGenerator->m_rocks.at(i);
            if (!rock) {
                continue;
            }

            // 检查石头是否已在本帧中被标记为删除 (安全措施)
            bool rockAlreadyMarkedForDeletion = false;
            for (IPhysicsObject* deletedObj : m_objectsToDeleteThisFrame) {
                if (rock == deletedObj) {
                    rockAlreadyMarkedForDeletion = true;
                    break;
                }
            }
            if (rockAlreadyMarkedForDeletion) {
                continue;
            }

            if (player->collidesWithItem(rock)) {
                player->checkHitRock(rock); // 调用修改后的方法，仅处理玩家状态

                // 从场景中移除石头
                if (rock->scene()) {
                    rock->scene()->removeItem(rock);
                }
                // 从物理系统中注销石头
                PhysicsSystem::instance().unregisterObject(rock);
                
                // 从地形生成器的石头列表中移除
                GTerrainGenerator->m_rocks.removeAt(i);

                // 将石头添加到本帧的待删除列表
                if (!m_objectsToDeleteThisFrame.contains(rock)) {
                    m_objectsToDeleteThisFrame.append(rock);
                }
                
                break; // 处理完一次碰撞即可
            }
        }
    }

    if (player) {
        // 用于并行处理的lambda
        auto checkRockCollision = [this, player](RockEntity* rock) {
            if (!rock) return;

            // 检查石头是否已在本帧中被标记为删除
            bool rockAlreadyMarkedForDeletion = false;
            for (IPhysicsObject* deletedObj : m_objectsToDeleteThisFrame) {
                if (rock == deletedObj) {
                    rockAlreadyMarkedForDeletion = true;
                    break;
                }
            }
            if (rockAlreadyMarkedForDeletion) return;

            if (player->collidesWithItem(rock)) {
                // 线程安全地处理碰撞结果
                QMetaObject::invokeMethod(this, [this, player, rock]() {
                    player->checkHitRock(rock);
                    if (rock->scene()) {
                        rock->scene()->removeItem(rock);
                    }
                    PhysicsSystem::instance().unregisterObject(rock);
                    GTerrainGenerator->m_rocks.removeOne(rock);
                    if (!m_objectsToDeleteThisFrame.contains(rock)) {
                        m_objectsToDeleteThisFrame.append(rock);
                    }
                }, Qt::QueuedConnection);
            }
        };

        // QtConcurrent 并行处理所有石头
        QtConcurrent::blockingMap(GTerrainGenerator->m_rocks, checkRockCollision);
    }
}

// 计算动态地面检测容差
qreal GameScene::calculateGroundTolerance(qreal speed, qreal slope, qreal forwardSlope, qreal verticalSpeed) {
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
bool GameScene::shouldMaintainGrounded(bool currentlyGrounded, qreal slope, qreal verticalSpeed, qreal horizontalSpeed) {
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
bool GameScene::shouldTakeoff(qreal backSlope, qreal currentSlope, qreal forwardSlope, qreal speed) {
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
void GameScene::handleTakeoff(IPhysicsObject* obj, qreal slope, qreal speed) {
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
void GameScene::updateSlopeForce(IPhysicsObject* obj, qreal slope, qreal speed) {
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
void GameScene::updateEntityRotation(BasePhysicsEntity* entity, bool onGround, qreal slope) {
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

// 新增：处理得分的槽函数
void GameScene::onGetScore(int points)
{
    // 应用分数奖励倍数
    int adjustedPoints = static_cast<int>(points * award_score);
    // 增加玩家得分
    score += adjustedPoints;

    qDebug() << "玩家得分: " << score << " (奖励倍数: " << award_score << ")";
}

// 显示游戏结束对话框
void GameScene::showGameOverDialog() {
    QDialog dialog;
    dialog.setWindowTitle("游戏结束");
    dialog.setModal(true);
    dialog.setFixedSize(350, 260);
    dialog.setStyleSheet(
        "QDialog { background: #f8fafd; border-radius: 18px; }"
        "QLabel { font-size: 20px; color: #333; }"
        "QPushButton {"
        "  min-width: 120px; min-height: 36px; font-size: 18px;"
        "  border-radius: 8px; background: #e0e7ef; color: #222;"
        "  margin: 8px 0;"
        "}"
        "QPushButton:hover { background: #b6d0f7; }"
    );

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(18);
    layout->setContentsMargins(30, 30, 30, 30);
    qreal secs = GElapsedTimer.elapsed() / 1000.0;

    QString iniPath = QCoreApplication::applicationDirPath() + "/game_record.ini";
    QSettings settings(iniPath, QSettings::IniFormat);
    qreal bestSecs = settings.value("General/bestTime", 0.0).toDouble();
    if (secs > bestSecs) {
        bestSecs = secs;
        settings.setValue("General/bestTime", bestSecs);
    }

    QLabel* gameOverLabel = new QLabel("GAME OVER");
    gameOverLabel->setAlignment(Qt::AlignCenter);
    gameOverLabel->setStyleSheet("font-size: 35px; font-weight: bold; color: #d32f2f; letter-spacing: 2px;");
    layout->addWidget(gameOverLabel);
    QLabel* title = new QLabel("游戏结束");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 26px; font-weight: bold; color: #1976d2;");
    layout->addWidget(title);

    QLabel* timeLabel = new QLabel(QString("本次游戏时长：%1 秒").arg(QString::number(secs, 'f', 2)));
    timeLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(timeLabel);

    QLabel* bestLabel = new QLabel(QString("历史最佳：%1 秒").arg(QString::number(bestSecs, 'f', 2)));
    bestLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(bestLabel);

    QPushButton* retryBtn = new QPushButton("再来一次");
    QPushButton* exitBtn = new QPushButton("退出游戏");
    retryBtn->setCursor(Qt::PointingHandCursor);
    exitBtn->setCursor(Qt::PointingHandCursor);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(retryBtn);
    btnLayout->addWidget(exitBtn);
    layout->addLayout(btnLayout);

    connect(retryBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(exitBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    // 居中显示（Qt6 推荐写法）
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        QPoint center = screenGeometry.center() - QPoint(dialog.width() / 2, dialog.height() / 2);
        dialog.move(center);
    }

    int result = dialog.exec();
    if (result == QDialog::Accepted) {
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
        initialize();
    } else {
        GTimer.stop();
        if (m_avalancheThread) {
            m_avalancheThread->stop();
            m_avalancheThread->wait();
            delete m_avalancheThread;
            m_avalancheThread = nullptr;
        }
        // 可选：清理其它资源
        qApp->quit();
    }
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

// 新增：创建场景关键元素
void GameScene::createSceneItems()
{

    // 地形生成器
    GTerrainGenerator = new TerrainGenerator(this, this);

    // 暂停文本和遮罩
    GPauseText = addText("", QFont("Arial", 24, QFont::Bold));
    GPauseText->setDefaultTextColor(Qt::white);
    GPauseText->setZValue(1001);
    GPauseText->hide();
    GPauseOverlay = new QGraphicsRectItem();
    GPauseOverlay->setZValue(1000);
    addItem(GPauseOverlay);
    GPauseOverlay->hide();

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
            this, [this]() { avalanche->applyThreadResults(); });
    m_avalancheThread->start();
}