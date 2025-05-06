#include "mainwindow.h"
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 设置窗口大小和标题
    resize(800, 600);
    setWindowTitle("无限滚动地形演示");
    
    // 创建视图和场景
    m_view = new QGraphicsView(this);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setBackgroundBrush(QBrush(QColor(135, 206, 235))); // 天空蓝
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    m_scene = new GameScene(this);
    m_view->setScene(m_scene);
    
    // 设置视图为中心部件
    setCentralWidget(m_view);
    
    // 初始化场景
    m_scene->initialize();
    
    // 将焦点设置到视图上，以便接收键盘事件
    m_view->setFocus();
}

MainWindow::~MainWindow()
{
}