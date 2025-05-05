#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QTimer>
#include <QKeyEvent>
#include "player.h"
#include "terraingenerator.h"

class GameScene : public QGraphicsScene
{
    Q_OBJECT
    
public:
    GameScene(QObject *parent = nullptr);
    ~GameScene();
    
    void initialize();
    
protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    
private slots:
    void update();
    
private:
    Player *m_player;
    TerrainGenerator *m_terrainGenerator;
    QTimer m_timer;

    void updatePlayerHeight();
    void centerViewOnPlayer();
};

#endif // GAMESCENE_H