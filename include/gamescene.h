#pragma once

#include <QGraphicsScene>
#include <QTimer>
#include <QKeyEvent>
#include "player.h"
#include "terraingenerator.h"
#include "avalanche.h"
#include "avalancheupdatethread.h"
#include <QList> // 添加 QList 头文件
#include "physical.h" // IPhysicsObject 定义
#include "uimanager.h" // 添加 UIManager 头文件
#include "collisionhandler.h" // 添加 CollisionHandler 头文件

class GameScene : public QGraphicsScene
{
    Q_OBJECT

public:
    enum GameState { Running, Paused };
    explicit GameScene(QObject *parent = nullptr);
    ~GameScene();
    
    void initialize();
    void showGameOverDialog();


signals:
    // 新增获得分数的信号
    void getscore(int points);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void update();

    void initialPlayerPosition();

    // 新增处理得分的槽函数
    void onGetScore(int points);
    
    // 暂停/继续游戏
    void togglePause();

private:
    void createSceneItems();  // 创建场景对象
    void resetGameState();    // 重置游戏状态

    GameState GState;
    Player *Gplayer;
    TerrainGenerator *GTerrainGenerator;
    QTimer GTimer;
    Avalanche* avalanche;

    // 新增得分和奖励倍数字段
    int score;                // 玩家当前得分
    double award_speed;       // 速度奖励倍数
    double award_score;       // 分数奖励倍数

    qreal m_avalancheElapsed = 0;
    const qreal m_avalancheInterval = 0.02; // 雪崩每0.1秒刷新一次

    void centerViewOnPlayer();

    QElapsedTimer GElapsedTimer;

    UIManager* m_uiManager;
    CollisionHandler* m_collisionHandler; // 碰撞处理器

    AvalancheUpdateThread* m_avalancheThread;  // 雪崩更新线程
    QList<IPhysicsObject*> m_objectsToDeleteThisFrame; // 存储本帧待删除的物理对象的列表
};

