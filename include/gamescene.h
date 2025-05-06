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
    void update();
    
private:
    GameState GState;
    Player *Gplayer;
    TerrainGenerator *GTerrainGenerator;
    QTimer GTimer;

    void updatePlayerHeight();
    void centerViewOnPlayer();

    void togglePause();

    QElapsedTimer GElapsedTimer;


    //ui区域
    QGraphicsTextItem *GPauseText;
    QPushButton *pauseButton;

};

