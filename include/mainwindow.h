#pragma once
#include <QMainWindow>
#include <QGraphicsView>
#include "gamescene.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    MainWindow(const QString& username, QWidget *parent = nullptr);
    ~MainWindow();

private:
    QGraphicsView *m_view;
    GameScene *m_scene;
    QString m_username;

    // 添加这个私有方法声明
    void setupWindow();
};