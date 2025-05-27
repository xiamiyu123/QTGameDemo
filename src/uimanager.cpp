#include "uimanager.h"
#include <QGraphicsView>
#include <QApplication>
#include <QScreen>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QCoreApplication>
#include <QRadialGradient>
#include <QStyleOptionGraphicsItem>

UIManager::UIManager(QGraphicsScene* scene, QObject* parent)
    : QObject(parent)
    , m_scene(scene)
    , m_pauseText(nullptr)
    , m_pauseButton(nullptr)
    , m_pauseOverlay(nullptr)
    , m_warningButton(nullptr)
{
}

UIManager::~UIManager()
{
    cleanup();
}

void UIManager::initialize()
{
    // 创建暂停相关UI元素
    createPauseElements();
    
    // 创建警告UI元素
    createWarningElements();
    
    // 更新初始UI位置
    updateUI();
}

void UIManager::cleanup()
{
    // 移除UI元素
    if (m_pauseButton) {
        m_pauseButton->setParent(nullptr);
        delete m_pauseButton;
        m_pauseButton = nullptr;
    }
    
    if (m_warningButton) {
        m_warningButton->setParent(nullptr);
        delete m_warningButton;
        m_warningButton = nullptr;
    }
    
    // 场景会自动处理其中的项目
    m_pauseText = nullptr;
    m_pauseOverlay = nullptr;
}

void UIManager::createPauseElements()
{
    // 创建暂停文本
    m_pauseText = m_scene->addText("", QFont("Arial", 24, QFont::Bold));
    m_pauseText->setDefaultTextColor(Qt::white);
    m_pauseText->setZValue(1001);
    m_pauseText->hide();
    
    // 创建暂停遮罩
    m_pauseOverlay = new QGraphicsRectItem();
    m_pauseOverlay->setZValue(1000);
    m_scene->addItem(m_pauseOverlay);
    m_pauseOverlay->hide();
    
    // 创建暂停按钮
    m_pauseButton = new QPushButton();
    setupButtonStyle(m_pauseButton, ":/resource/images/icons/pause.svg");
    m_pauseButton->setIconSize(QSize(50, 50));
    connect(m_pauseButton, &QPushButton::pressed, this, &UIManager::pauseToggled);
}

void UIManager::createWarningElements()
{
    // 创建警告按钮
    m_warningButton = new QPushButton();
    setupButtonStyle(m_warningButton, ":/resource/images/icons/warning.png");
    m_warningButton->setIconSize(QSize(50, 50));
    m_warningButton->hide();
}

void UIManager::setupButtonStyle(QPushButton* button, const QString& iconPath, bool transparent)
{
    if (!button) return;
    
    button->setIcon(QIcon(iconPath));
    button->setFocusPolicy(Qt::NoFocus);
    
    if (transparent) {
        button->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
        button->setAttribute(Qt::WA_TranslucentBackground);
    }
}

void UIManager::updateUI()
{
    QGraphicsView* view = getView();
    if (!view) return;
    
    // 更新暂停按钮位置
    if (m_pauseButton) {
        m_pauseButton->setParent(view->viewport());
        QRect vp = view->viewport()->rect();
        m_pauseButton->setGeometry(vp.width() - 50 - 10, 10, 50, 50);
        m_pauseButton->show();
    }
    
    // 更新暂停文字位置
    if (m_pauseText && m_pauseText->isVisible()) {
        QRectF viewSceneRect = view->mapToScene(view->viewport()->rect()).boundingRect();
        QRectF textRect = m_pauseText->boundingRect();
        // 文字居中到视口
        m_pauseText->setPos(
            viewSceneRect.center().x() - textRect.width() / 2,
            viewSceneRect.center().y() - textRect.height() / 2
        );
    }
    
    // 更新警告按钮位置
    if (m_warningButton) {
        m_warningButton->setParent(view->viewport());
        m_warningButton->setGeometry(10, 10, 
                                     m_warningButton->iconSize().width(), 
                                     m_warningButton->iconSize().height());
    }
}

void UIManager::showPauseOverlay(bool show, int score)
{
    QGraphicsView* view = getView();
    if (!view) return;
    
    if (show) {
        // 创建渐变效果 - 从深蓝色边缘到浅蓝色中心的径向渐变
        QRectF viewRect = view->mapToScene(view->viewport()->rect()).boundingRect();
        
        // 设置遮罩区域覆盖整个可视区域
        m_pauseOverlay->setRect(viewRect);
        
        // 创建从边缘深蓝到中心浅蓝的径向渐变
        QRadialGradient gradient(viewRect.center(), qMax(viewRect.width(), viewRect.height()) / 2);
        gradient.setColorAt(0.0, QColor(100, 180, 255, 180));   // 中心浅蓝色，半透明
        gradient.setColorAt(1.0, QColor(10, 50, 120, 230));     // 边缘深蓝色，更不透明
        
        m_pauseOverlay->setBrush(gradient);
        m_pauseOverlay->setPen(Qt::NoPen);  // 无边框
        m_pauseOverlay->show();
        
        // 更新暂停文字内容，显示分数
        m_pauseText->setPlainText(QString("游戏已暂停\n\n当前分数：%1 分").arg(score));
        m_pauseText->show();
        
        // 更改暂停按钮图标
        if (m_pauseButton) {
            m_pauseButton->setIcon(QIcon(":/resource/images/icons/play.svg"));
        }
        
        // 立即更新一次UI以定位暂停文字
        updateUI();
    }
    else {
        m_pauseText->hide();
        m_pauseOverlay->hide();
        
        // 更改暂停按钮图标
        if (m_pauseButton) {
            m_pauseButton->setIcon(QIcon(":/resource/images/icons/pause.svg"));
        }
    }
}

void UIManager::showWarningIndicator(bool show, qreal distance)
{
    if (!m_warningButton) return;
    
    if (show) {
        int size;
        if (distance < 800) {
            size = 80;
        } else if (distance < 1200 && distance >= 800) {
            size = 50 + int((1200 - distance) / 400.0 * 30);
        } else {
            size = 50;
        }
        
        m_warningButton->show();
        m_warningButton->setIconSize(QSize(size, size));
        m_warningButton->setGeometry(10, 10, size, size);
    }
    else {
        m_warningButton->hide();
    }
}

void UIManager::showGameOverDialog(int score, const std::function<void()>& onRetry, const std::function<void()>& onExit)
{
    QDialog dialog;
    dialog.setWindowTitle("游戏结束");
    dialog.setModal(true);
    dialog.setFixedSize(350, 260);
    dialog.setStyleSheet(
        "QDialog { background: #f8fafd; border-radius: 18px; }"
        "QLabel { font-size: 20px; color: #333; }"
        "QPushButton {"
        "  min-width: 120px; min-height: 36px; font-size: 18px;"
        "  border-radius: 8px; background: #e0e7ef; color: #222;"
        "  margin: 8px 0;"
        "}"
        "QPushButton:hover { background: #b6d0f7; }"
    );

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(18);
    layout->setContentsMargins(30, 30, 30, 30);

    // 读取历史最高分
    QString iniPath = QCoreApplication::applicationDirPath() + "/game_record.ini";
    QSettings settings(iniPath, QSettings::IniFormat);
    int bestScore = settings.value("General/bestScore", 0).toInt();
    if (score >= bestScore) {
        bestScore = score;
        settings.setValue("General/bestScore", bestScore);
    }

    QLabel* gameOverLabel = new QLabel("GAME OVER");
    gameOverLabel->setAlignment(Qt::AlignCenter);
    gameOverLabel->setStyleSheet("font-size: 35px; font-weight: bold; color: #d32f2f; letter-spacing: 2px;");
    layout->addWidget(gameOverLabel);

    QLabel* title = new QLabel("游戏结束");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 26px; font-weight: bold; color: #1976d2;");
    layout->addWidget(title);

    QLabel* scoreLabel = new QLabel(QString("本次得分：%1 分").arg(score));
    scoreLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(scoreLabel);

    QLabel* bestLabel = new QLabel(QString("历史最高：%1 分").arg(bestScore));
    bestLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(bestLabel);

    QPushButton* retryBtn = new QPushButton("再来一次");
    QPushButton* exitBtn = new QPushButton("退出游戏");
    retryBtn->setCursor(Qt::PointingHandCursor);
    exitBtn->setCursor(Qt::PointingHandCursor);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(retryBtn);
    btnLayout->addWidget(exitBtn);
    layout->addLayout(btnLayout);

    connect(retryBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(exitBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        QPoint center = screenGeometry.center() - QPoint(dialog.width() / 2, dialog.height() / 2);
        dialog.move(center);
    }

    int result = dialog.exec();
    if (result == QDialog::Accepted) {
        if (onRetry) onRetry();
    } else {
        if (onExit) onExit();
    }
}

QGraphicsView* UIManager::getView() const
{
    if (!m_scene || m_scene->views().isEmpty()) {
        return nullptr;
    }
    return m_scene->views().first();
}
