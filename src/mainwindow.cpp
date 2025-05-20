#include "mainwindow.h"
#include <QKeyEvent>
#include <QOpenGLWidget> // 添加OpenGL支持

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //打开硬件加速等基本优化
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    // 窗口基本设置：设置初始尺寸与标题
    //设置全屏
    setWindowState(Qt::WindowMaximized);
    setWindowTitle("滑雪大冒险");
    // 初始化视图 (QGraphicsView)，并配置渲染与背景
    m_view = new QGraphicsView(this);

    // 设置OpenGL视口以启用硬件加速
    QOpenGLWidget *glWidget = new QOpenGLWidget();
    QSurfaceFormat format;
    format.setSamples(4); // 多重采样抗锯齿
    format.setSwapInterval(1); // 垂直同步
    glWidget->setFormat(format);
    m_view->setViewport(glWidget);

    // 设置视图的缓存和优化模式
    m_view->setCacheMode(QGraphicsView::CacheBackground);
    m_view->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);

    m_view->setRenderHint(QPainter::Antialiasing); // 开启抗锯齿
    m_view->setRenderHint(QPainter::SmoothPixmapTransform, true); // 平滑图像缩放
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

