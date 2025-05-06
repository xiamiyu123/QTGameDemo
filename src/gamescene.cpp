#include "gamescene.h"
#include <QGraphicsView>
#include <QtSvg>
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
    Gplayer->setPos(500, 0);
    updatePlayerHeight();
    
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

void GameScene::update() // 每16ms调用一次 更新游戏状态
{
    // 更新玩家位置
    Gplayer->update();
    
    // 更新玩家与地形的关系
    updatePlayerHeight();
    
    // 更新地形生成
    GTerrainGenerator->updateTerrain(Gplayer->x());
    
    // 让视图跟随玩家
    centerViewOnPlayer();

    // 更新ui控件
    updateUI();
}

void GameScene::updatePlayerHeight() {
    // 让玩家站在地形上
    // TODO: 未实现重力
    qreal terrainHeight = GTerrainGenerator->getTerrainHeight(Gplayer->x() + Gplayer->rect().width() / 2);
    Gplayer->setY(terrainHeight - Gplayer->rect().height());
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