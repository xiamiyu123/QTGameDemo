#include "mainwindow.h"
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 窗口基本设置：设置初始尺寸与标题
    resize(800, 600); // 宽800，高600
    setWindowTitle("无限滚动地形演示");
    // 初始化视图 (QGraphicsView)，并配置渲染与背景
    m_view = new QGraphicsView(this);
    m_view->setRenderHint(QPainter::Antialiasing); // 开启抗锯齿
    m_view->setBackgroundBrush(QBrush(QColor(135, 206, 235))); // 天空蓝背景
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 禁用水平滚动条
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);   // 禁用垂直滚动条
    m_scene = new GameScene(this);
    m_view->setScene(m_scene);

    // 将图形视图设置为主窗口的中心部件
    setCentralWidget(m_view);

    // 初始化游戏场景及其定时器
    m_scene->initialize();

    // 将键盘焦点切换到视图，以接收用户输入
    m_view->setFocus();
}

MainWindow::~MainWindow()
{
}