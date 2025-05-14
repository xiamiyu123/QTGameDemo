#pragma once

#include <QGraphicsScene>
#include <QTimer>
#include <QKeyEvent>
#include "player.h"
#include "terraingenerator.h"
#include <QPushButton>

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

    qreal calculateGroundTolerance(qreal speed, qreal slope, qreal forwardSlope, qreal verticalSpeed);

    bool shouldMaintainGrounded(bool currentlyGrounded, qreal slope, qreal verticalSpeed, qreal horizontalSpeed);

    bool shouldTakeoff(qreal backSlope, qreal currentSlope, qreal forwardSlope, qreal speed);

    void handleTakeoff(IPhysicsObject *obj, qreal slope, qreal speed);

    void updateSlopeForce(IPhysicsObject *obj, qreal slope, qreal speed);

    void updateEntityRotation(BasePhysicsEntity *entity, bool onGround, qreal slope);

    void update();

    void initialPlayerPosition();

private:
    GameState GState;
    Player *Gplayer;
    TerrainGenerator *GTerrainGenerator;
    QTimer GTimer;

    void updatePlayerHeight();
    void centerViewOnPlayer();

    void togglePause();

    QElapsedTimer GElapsedTimer;

    QTime GLastUpdateTime; // 上次更新时间


    //ui区域
    QGraphicsTextItem *GPauseText;
    QPushButton *pauseButton;

};

