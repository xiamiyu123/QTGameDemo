#pragma once
#include <QGraphicsRectItem>
#include <QObject>
#include <QKeyEvent>

class Player : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

public:
    Player(QGraphicsItem *parent = nullptr);
    
    void move(qreal dx, qreal dy);
    void update();
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    
private:
    qreal m_velocityX;
    qreal m_velocityY;
    bool m_movingLeft;
    bool m_movingRight;
};
