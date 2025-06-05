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

signals:
    // 请求返回主菜单的信号
    void requestReturnToMainMenu();

protected:
    // 重写窗口关闭事件
    void closeEvent(QCloseEvent* event) override;

private:
    QGraphicsView *m_view;
    GameScene *m_scene;
    QString m_username;

    // 添加这个私有方法声明
    void setupWindow();
};