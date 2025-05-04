#include "gamescene.h"
#include <QGraphicsView>

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent)
{
    // 设置场景大小（足够大以容纳滚动地形）
    setSceneRect(-10000, -1000, 20000, 2000);
    
    // 创建地形生成器
    m_terrainGenerator = new TerrainGenerator(this, this);
    
    // 创建玩家
    m_player = new Player();
    addItem(m_player);
    
    // 设置游戏循环定时器
    connect(&m_timer, &QTimer::timeout, this, &GameScene::update);
    m_timer.setInterval(16); // 约60fps
}

GameScene::~GameScene()
{
}

void GameScene::initialize()
{
    // 初始化地形
    m_terrainGenerator->initialize();
    
    // 将玩家放置在适当位置
    m_player->setPos(0, 0);
    updatePlayerOnTerrain();
    
    // 启动游戏循环
    m_timer.start();
}

void GameScene::keyPressEvent(QKeyEvent *event)
{
    // 把键盘事件传递给玩家
    m_player->keyPressEvent(event);
    QGraphicsScene::keyPressEvent(event);
}

void GameScene::keyReleaseEvent(QKeyEvent *event)
{
    // 把键盘事件传递给玩家
    m_player->keyReleaseEvent(event);
    QGraphicsScene::keyReleaseEvent(event);
}

void GameScene::update()
{
    // 更新玩家位置
    m_player->update();
    
    // 更新玩家与地形的关系
    updatePlayerOnTerrain();
    
    // 更新地形生成
    m_terrainGenerator->updateTerrain(m_player->x());
    
    // 让视图跟随玩家
    centerViewOnPlayer();
}

void GameScene::updatePlayerOnTerrain()
{
    // 让玩家站在地形上
    qreal terrainHeight = m_terrainGenerator->getTerrainHeight(m_player->x() + m_player->rect().width() / 2);
    m_player->setY(terrainHeight - m_player->rect().height());
}

void GameScene::centerViewOnPlayer()
{
    if (views().isEmpty()) {
        return;
    }
    
    QGraphicsView *view = views().first();
    
    // 计算视图中间位置
    qreal viewCenterX = m_player->x() + 200; // 玩家位置偏右一点
    qreal viewCenterY = m_player->y();
    
    // 设置视图中心
    view->centerOn(viewCenterX, viewCenterY);
}