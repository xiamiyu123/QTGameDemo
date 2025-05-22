#pragma once

#include <QGraphicsScene>
#include <QTimer>
#include <QKeyEvent>
#include "player.h"
#include "terraingenerator.h"
#include <QPushButton>
#include "avalanche.h"
#include "avalancheupdatethread.h"
#include <QList> // 添加 QList 头文件
#include "physical.h" // IPhysicsObject 定义

class GameScene : public QGraphicsScene
{
    Q_OBJECT

public:
    enum GameState { Running, Paused };
    explicit GameScene(QObject *parent = nullptr);
    ~GameScene();
    
    void initialize();

signals:
    // 新增获得分数的信号
    void getscore(int points);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void updateUI();

    void handlePhysicsObjectCollision(IPhysicsObject *obj);

    qreal calculateGroundTolerance(qreal speed, qreal slope, qreal forwardSlope, qreal verticalSpeed);

    bool shouldMaintainGrounded(bool currentlyGrounded, qreal slope, qreal verticalSpeed, qreal horizontalSpeed);

    bool shouldTakeoff(qreal backSlope, qreal currentSlope, qreal forwardSlope, qreal speed);

    void handleTakeoff(IPhysicsObject *obj, qreal slope, qreal speed);

    void updateSlopeForce(IPhysicsObject *obj, qreal slope, qreal speed);

    void updateEntityRotation(BasePhysicsEntity *entity, bool onGround, qreal slope);

    void update();

    void initialPlayerPosition();

    // 新增处理得分的槽函数
    void onGetScore(int points);

private:
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

    void updatePlayerHeight();
    void centerViewOnPlayer();

    void togglePause();

    QElapsedTimer GElapsedTimer;

    QTime GLastUpdateTime; // 上次更新时间


    //ui区域
    QGraphicsTextItem *GPauseText;
    QPushButton *pauseButton;
    AvalancheUpdateThread* m_avalancheThread;  // 雪崩更新线程
    QList<IPhysicsObject*> m_objectsToDeleteThisFrame; // 新增：用于存储本帧待删除的对象
};

