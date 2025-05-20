#pragma once

#include <QGraphicsScene>
#include <QTimer>
#include <QKeyEvent>
#include "player.h"
#include "terraingenerator.h"
#include <QPushButton>
#include "avalanche.h"

class GameScene : public QGraphicsScene
{
    Q_OBJECT
    
public:
    enum GameState { Running, Paused };
    GameScene(QObject *parent = nullptr);
    ~GameScene();
    
    void initialize();
    
protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    
private slots:
    void updateUI();

    void handlePhysicsObjectCollision(IPhysicsObject *obj);

    void handleGroundedState(IPhysicsObject *obj, qreal terrainSlope);

    void update();

    void initialPlayerPosition();

private:
    GameState GState;
    Player *Gplayer;
    TerrainGenerator *GTerrainGenerator;
    QTimer GTimer;
    Avalanche* avalanche;

    void updatePlayerHeight();
    void centerViewOnPlayer();

    void togglePause();

    QElapsedTimer GElapsedTimer;

    QTime GLastUpdateTime; // 上次更新时间


    //ui区域
    QGraphicsTextItem *GPauseText;
    QPushButton *pauseButton;

};

