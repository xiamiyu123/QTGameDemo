#pragma once
#include <QMainWindow>
#include <QGraphicsView>
#include "gamescene.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QGraphicsView *m_view;
    GameScene *m_scene;
};
