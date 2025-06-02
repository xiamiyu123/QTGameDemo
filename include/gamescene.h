#pragma once

#include <QGraphicsScene>
#include <QTimer>
#include <QKeyEvent>
#include "player.h"
#include "terraingenerator.h"
#include "avalanche.h"
#include "avalancheupdatethread.h"
#include <QList>
#include "physical.h" 
#include "uimanager.h" 
#include "collisionhandler.h"
#include "debuglogger.h"
#include "npcentity.h"
#include <QGraphicsSceneWheelEvent>
#include <vector>
#include <memory>

class GameScene : public QGraphicsScene
{
    Q_OBJECT

public:
    enum GameState { Running, Paused, GameOver };
    explicit GameScene(QObject *parent = nullptr);
    ~GameScene();
    
    void initialize();
    void showGameOverDialog();
    void checkPlayerProgressScore();


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
    void onBackFlipSuccess(int points, const QString& message);

      // 暂停/继续游戏
    void togglePause();

    // NPC管理槽函数
    void spawnNPC();
    void spawnRandomNPC(const QPointF& position);

private:
    void createSceneItems();  // 创建场景对象
    void resetGameState();    // 重置游戏状态
    // NPC管理方法
    void initializeNPCSystem();      // 初始化NPC系统
    void updateAllNPCs(float deltaTime);  // 更新所有NPC
    void cleanupNPCs();               // 清理需要销毁的NPC
    void removeOffscreenNPCs();       // 移除离屏幕太远的NPC
    QPointF getNPCSpawnPosition();    // 获取NPC生成位置

    GameState GState;
    Player *Gplayer;
    TerrainGenerator *GTerrainGenerator;
    QTimer GTimer;
    Avalanche* avalanche;

    // 新增得分和奖励倍数字段
    int score;                // 玩家当前得分
    double award_speed;       // 速度奖励倍数
    double award_score;       // 分数奖励倍数

    qreal m_lastScoredPositionX;  // 上次得分时玩家的X位置
    qreal m_scoreDistance;        // 每多少距离得分一次

    qreal m_avalancheElapsed = 0;
    const qreal m_avalancheInterval = 0.02; // 雪崩每0.1秒刷新一次

    void centerViewOnPlayer();

    QElapsedTimer GElapsedTimer;

    UIManager* m_uiManager;
    CollisionHandler* m_collisionHandler; // 碰撞处理器
    AvalancheUpdateThread* m_avalancheThread;  // 雪崩更新线程
    QList<IPhysicsObject*> m_objectsToDeleteThisFrame; // 存储本帧待删除的物理对象的列表

    // === NPC管理系统 ===
    std::vector<std::unique_ptr<NPCEntity>> m_activeNPCs;  // 活跃的NPC列表
    QTimer* m_npcSpawnTimer;                                // NPC生成定时器
    qreal m_npcSpawnDistance;                               // NPC生成距离
    int m_maxNPCCount;                                      // 最大NPC数量
    qreal m_npcCleanupDistance;                             // NPC清理距离
    void wheelEvent(QGraphicsSceneWheelEvent *event) override {
        // 阻止滚轮事件继续传递到 QGraphicsView
        event->accept();// 滚轮事件被拦截防止不会引起视图缩放
    }

    void test()
    {
        // 用于测试的构造时函数
        //
    }

    void clearGameObjects(); // 清理游戏对象但保留UI元素
};

