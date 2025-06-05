#include "uimanager.h"
#include "gamescene.h"
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
#include <qfontdatabase.h>
#include <QRadialGradient>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QTimer>
#include <QDateTime>
#include <algorithm>

UIManager::UIManager(QGraphicsScene* scene, QObject* parent)
    : QObject(parent)
    , m_scene(scene)
    , m_pauseText(nullptr)
    , m_pauseButton(nullptr)
    , m_pauseOverlay(nullptr)    , m_warningButton(nullptr)    , m_npcCooldownContainer(nullptr)
    , m_npcCooldownProgress(nullptr)
    , m_fallRecoveryContainer(nullptr)
    , m_fallRecoveryProgress(nullptr)
    , m_flipBoostContainer(nullptr)
    , m_flipBoostProgress(nullptr)
    , m_scoreLabel(nullptr)
    , m_scorePopupLabel(nullptr)
    , m_popupTimer(nullptr)
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

    // 创建得分标签
    createScoreLabel();

    // 创建NPC拾取冷却进度条
    createNPCCooldownElements();

    // 创建摔倒恢复进度条
    createFallRecoveryElements();

    // 创建空翻加速进度条
    createFlipBoostElements();

    // 创建得分倍率条
    createScoreMultiplierElements();

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

    if (m_npcCooldownContainer) {
        m_npcCooldownContainer->setParent(nullptr);
        delete m_npcCooldownContainer;
        m_npcCooldownContainer = nullptr;
    }

    if (m_fallRecoveryContainer) {
        m_fallRecoveryContainer->setParent(nullptr);
        delete m_fallRecoveryContainer;
        m_fallRecoveryContainer = nullptr;
    }

    if (m_flipBoostContainer) {
        m_flipBoostContainer->setParent(nullptr);
        delete m_flipBoostContainer;
        m_flipBoostContainer = nullptr;
    }

    // 清理得分倍率条
    if (m_scoreMultiplierContainer) {
        m_scoreMultiplierContainer->setParent(nullptr);
        delete m_scoreMultiplierContainer;
        m_scoreMultiplierContainer = nullptr;
        m_scoreMultiplierBar = nullptr;     // 子部件会随父部件一起删除
        m_scoreMultiplierLabel = nullptr;   // 子部件会随父部件一起删除
    }

    // 清理其他UI元素
    if (m_scoreLabel) {
        m_scoreLabel->setParent(nullptr);
        delete m_scoreLabel;
        m_scoreLabel = nullptr;
    }

    if (m_scorePopupLabel) {
        m_scorePopupLabel->setParent(nullptr);
        delete m_scorePopupLabel;
        m_scorePopupLabel = nullptr;
    }

    // 场景会自动处理其中的项目
    m_pauseText = nullptr;
    m_pauseOverlay = nullptr;
    m_npcCooldownProgress = nullptr;
    m_fallRecoveryProgress = nullptr;
    m_flipBoostProgress = nullptr;
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

void UIManager::createNPCCooldownElements()
{
    // 创建进度条容器（背景）
    m_npcCooldownContainer = new QWidget();
    m_npcCooldownContainer->setFixedSize(200, 8);
    m_npcCooldownContainer->setStyleSheet(
        "background-color: rgba(50, 50, 50, 150);"
        "border: 1px solid white;"
        "border-radius: 4px;"
    );
    m_npcCooldownContainer->hide();

    // 创建进度条（前景）- 黄色
    m_npcCooldownProgress = new QWidget(m_npcCooldownContainer);
    m_npcCooldownProgress->setStyleSheet(
        "background-color: rgb(255, 193, 7);"  // 黄色进度条
        "border: none;"
        "border-radius: 3px;"
    );
    m_npcCooldownProgress->setGeometry(1, 1, 198, 6); // 留出边框空间
    m_npcCooldownProgress->hide();
}

void UIManager::createFallRecoveryElements()
{
    // 创建摔倒恢复进度条容器（背景）
    m_fallRecoveryContainer = new QWidget();
    m_fallRecoveryContainer->setFixedSize(200, 8);
    m_fallRecoveryContainer->setStyleSheet(
        "background-color: rgba(50, 50, 50, 150);"
        "border: 1px solid white;"
        "border-radius: 4px;"
    );
    m_fallRecoveryContainer->hide();

    // 创建摔倒恢复进度条（前景）- 红色
    m_fallRecoveryProgress = new QWidget(m_fallRecoveryContainer);
    m_fallRecoveryProgress->setStyleSheet(
        "background-color: rgb(220, 53, 69);"  // 红色进度条
        "border: none;"
        "border-radius: 3px;"
    );
    m_fallRecoveryProgress->setGeometry(1, 1, 198, 6); // 留出边框空间
    m_fallRecoveryProgress->hide();
}

void UIManager::createFlipBoostElements()
{
    // 创建空翻加速进度条容器（背景）
    m_flipBoostContainer = new QWidget();
    m_flipBoostContainer->setFixedSize(200, 8);
    m_flipBoostContainer->setStyleSheet(
        "background-color: rgba(50, 50, 50, 150);"
        "border: 1px solid white;"
        "border-radius: 4px;"
    );
    m_flipBoostContainer->hide();

    // 创建空翻加速进度条（前景）- 蓝色
    m_flipBoostProgress = new QWidget(m_flipBoostContainer);
    m_flipBoostProgress->setStyleSheet(
        "background-color: rgb(13, 110, 253);"  // 蓝色进度条
        "border: none;"
        "border-radius: 3px;"
    );
    m_flipBoostProgress->setGeometry(1, 1, 198, 6); // 留出边框空间
    m_flipBoostProgress->hide();
}

void UIManager::createScoreLabel()
{

    // 引入新字体
    int id = QFontDatabase::addApplicationFont(":/resource/fonts/1.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);

    // 创建分数标签
    QFont scoreFont(family, 40, QFont::Bold);
    m_scoreLabel = new QLabel("0");
    m_scoreLabel->setFont(scoreFont);
    m_scoreLabel->setStyleSheet("color: white; "
                                "border: none; "
                                "background: transparent; "
                                "padding: 6px 18px;");
    m_scoreLabel->setAlignment(Qt::AlignRight);

    // 添加临时得分提示标签
    QFont popupFont(family, 20, QFont::Bold);
    m_scorePopupLabel = new QLabel();
    m_scorePopupLabel->setFont(popupFont);
    m_scorePopupLabel->setStyleSheet("color: yellow; "
                                      "background: transparent; "
                                      "border: none;"
                                      "padding: 5px 10px;");
    m_scorePopupLabel->setAlignment(Qt::AlignRight);
    m_scorePopupLabel->hide();

    // 初始化计时器
    m_popupTimer = new QTimer(this);
    m_popupTimer->setSingleShot(true);
    connect(m_popupTimer, &QTimer::timeout, this, [this]() {
        if (m_scorePopupLabel) {
            m_scorePopupLabel->hide();
        }

        // 恢复分数标签为白色
        if (m_scoreLabel) {
            m_scoreLabel->setStyleSheet("color: white; "
                                      "border: none; "
                                      "background: transparent; "
                                      "padding: 6px 18px;");
        }
    });
}

void UIManager::createScoreMultiplierElements()
{
    // 引入字体
    int id = QFontDatabase::addApplicationFont(":/resource/fonts/1.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);

    // 创建容器（背景）
    m_scoreMultiplierContainer = new QWidget();
    m_scoreMultiplierContainer->setFixedSize(500, 30); // 从250改为500像素宽，加长为原来2倍
    m_scoreMultiplierContainer->setStyleSheet(
        "background-color: rgba(50, 50, 50, 150);"
        "border: 1px solid white;"
        "border-radius: 15px;"
    );

    // 创建进度条（前景）
    m_scoreMultiplierBar = new QWidget(m_scoreMultiplierContainer);
    m_scoreMultiplierBar->setStyleSheet(
        "background-color: rgba(255, 215, 0, 180);"  // 金色
        "border: none;"
        "border-radius: 14px;"
    );
    m_scoreMultiplierBar->setGeometry(1, 1, 1, 28); // 初始宽度为1

    // 创建标签
    QFont labelFont(family, 12, QFont::Bold);
    m_scoreMultiplierLabel = new QLabel("x1.0", m_scoreMultiplierContainer);
    m_scoreMultiplierLabel->setFont(labelFont);
    m_scoreMultiplierLabel->setStyleSheet("color: white; background: transparent;");
    m_scoreMultiplierLabel->setAlignment(Qt::AlignCenter);
    m_scoreMultiplierLabel->setGeometry(0, 0, 500, 30); // 从250改为500像素宽

    // 默认隐藏
    m_scoreMultiplierContainer->hide(); // 默认隐藏，倍率为1时不显示
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
    
    QRect vp = view->viewport()->rect();

    // 更新暂停按钮位置
    if (m_pauseButton) {
        m_pauseButton->setParent(view->viewport());
        m_pauseButton->setGeometry(vp.width() - 50 - 10, 10, 50, 50);
        m_pauseButton->show();
    }
    
    // 更新得分倍率条位置（与暂停按钮同高，视图中间）
    if (m_scoreMultiplierContainer) {
        m_scoreMultiplierContainer->setParent(view->viewport());
        int containerWidth = m_scoreMultiplierContainer->width();
        int containerHeight = m_scoreMultiplierContainer->height();
        // 水平居中，垂直位置与暂停按钮顶部对齐
        m_scoreMultiplierContainer->setGeometry(
            (vp.width() - containerWidth) / 2,  // 水平居中
            10,                                 // 与暂停按钮顶部对齐
            containerWidth,
            containerHeight
        );
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

    // 更新NPC冷却进度条位置（下移）
    if (m_npcCooldownContainer) {
        m_npcCooldownContainer->setParent(view->viewport());
        int barWidth = 200;
        int barHeight = 8;
        int bottomMargin = 100; // 距离底部的距离，下移了
        m_npcCooldownContainer->setGeometry(
            (vp.width() - barWidth) / 2,  // 水平居中
            vp.height() - bottomMargin,   // 距离底部
            barWidth,
            barHeight
        );
    }

    // 更新摔倒恢复进度条位置（在NPC进度条上方）
    if (m_fallRecoveryContainer) {
        m_fallRecoveryContainer->setParent(view->viewport());
        int barWidth = 200;
        int barHeight = 8;
        int bottomMargin = 80; // 距离底部的距离，在NPC进度条原位置
        m_fallRecoveryContainer->setGeometry(
            (vp.width() - barWidth) / 2,  // 水平居中
            vp.height() - bottomMargin,   // 距离底部
            barWidth,
            barHeight
        );
    }
    
    // 更新空翻加速进度条位置（在摔倒恢复进度条上方）
    if (m_flipBoostContainer) {
        m_flipBoostContainer->setParent(view->viewport());
        int barWidth = 200;
        int barHeight = 8;
        int bottomMargin = 120; // 距离底部的距离，在摔倒恢复进度条上方
        m_flipBoostContainer->setGeometry(
            (vp.width() - barWidth) / 2,  // 水平居中
            vp.height() - bottomMargin,   // 距离底部
            barWidth,
            barHeight
        );
    }

    // 更新分数标签位置
    if (m_scoreLabel) {
        m_scoreLabel->setParent(view->viewport());
        m_scoreLabel->setGeometry(vp.width() - 350, 5, 300, 82);
        m_scoreLabel->show();
    }
}

void UIManager::setScore(int score) {
    if (m_scoreLabel) {
        m_scoreLabel->setText(QString("%1").arg(score));
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

void UIManager::showNPCPickupCooldown(bool show, qreal progress)
{
    if (!m_npcCooldownContainer || !m_npcCooldownProgress) return;

    if (show) {
        // 确保进度条已经添加到视口
        QGraphicsView* view = getView();
        if (view) {
            m_npcCooldownContainer->setParent(view->viewport());
            updateUI(); // 更新位置
        }

        // 显示容器
        m_npcCooldownContainer->show();

        // 计算进度条宽度（剩余时间）
        qreal progressWidth = 198 * (1.0 - progress); // progress是0-1之间的值，表示已经过的时间比例
        m_npcCooldownProgress->setFixedWidth(qMax(0.0, progressWidth));
        m_npcCooldownProgress->show();
    }
    else {
        m_npcCooldownContainer->hide();
        m_npcCooldownProgress->hide();
    }
}

void UIManager::showFallRecovery(bool show, qreal progress)
{
    if (!m_fallRecoveryContainer || !m_fallRecoveryProgress) return;

    if (show) {
        // 确保进度条已经添加到视口
        QGraphicsView* view = getView();
        if (view) {
            m_fallRecoveryContainer->setParent(view->viewport());
            updateUI(); // 更新位置
        }

        // 显示容器
        m_fallRecoveryContainer->show();

        // 计算进度条宽度（剩余时间）
        qreal progressWidth = 198 * (1.0 - progress); // progress是0-1之间的值，表示已经过的时间比例
        m_fallRecoveryProgress->setFixedWidth(qMax(0.0, progressWidth));
        m_fallRecoveryProgress->show();
    }
    else {
        m_fallRecoveryContainer->hide();
        m_fallRecoveryProgress->hide();
    }
}

void UIManager::showFlipBoost(bool show, qreal progress)
{
    if (!m_flipBoostContainer || !m_flipBoostProgress) return;

    if (show) {
        // 确保进度条已经添加到视口
        QGraphicsView* view = getView();
        if (view) {
            m_flipBoostContainer->setParent(view->viewport());
            updateUI(); // 更新位置
        }

        // 显示容器
        m_flipBoostContainer->show();

        // 计算进度条宽度（剩余时间）
        qreal progressWidth = 198 * (1.0 - progress); // progress是0-1之间的值，表示已经过的时间比例
        m_flipBoostProgress->setFixedWidth(qMax(0.0, progressWidth));
        m_flipBoostProgress->show();
    }
    else {
        m_flipBoostContainer->hide();
        m_flipBoostProgress->hide();
    }
}

void UIManager::showGameOverDialog(int score, const std::function<void()>& onRetry, const std::function<void()>& onExit, bool updateLeaderboard)
{
    QGraphicsView* view = getView();
    if (!view) return;

    QRectF sceneRect = view->mapToScene(view->viewport()->rect()).boundingRect();

    // 卡片内容
    QWidget* card = new QWidget;
    card->setStyleSheet("background: #e3f2fd; border: 2px solid #1976d2; border-radius: 0px;");
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setSpacing(18);
    layout->setContentsMargins(30, 30, 30, 30);

    QLabel* title = new QLabel("GAME OVER");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 40px; font-weight: bold; color: #1976d2; border: none;");
    layout->addWidget(title);    QLabel* scoreLabel = new QLabel(QString("本次得分：<b style='color:#1976d2;'>%1</b> 分").arg(QString::number(score)));
    scoreLabel->setAlignment(Qt::AlignCenter);
    scoreLabel->setStyleSheet("font-size: 26px; color: #1565c0; border: none;");
    layout->addWidget(scoreLabel);    // 获取当前登录用户名 - 从主窗口标题中提取或从用户数据中获取
    QString currentUsername = getCurrentUsername();
    
    // 只在第一次显示游戏结束对话框时更新排行榜记录
    if (updateLeaderboard) {
        updateGlobalLeaderboard(currentUsername, score);
        updatePersonalLeaderboard(currentUsername, score);
    }
    
    // 获取最新的排行榜数据用于显示
    QList<ScoreRecord> globalTop10 = getGlobalTop10();
    QList<ScoreRecord> personalTop10 = getPersonalTop10(currentUsername);
    
    // 显示个人最高分（从个人前十名中取第一名）
    int personalBestScore = personalTop10.isEmpty() ? 0 : personalTop10.first().score;
    QLabel* personalBestLabel = new QLabel(QString("个人最高：<b style='color:#d32f2f;'>%1</b> 分").arg(QString::number(personalBestScore)));
    personalBestLabel->setAlignment(Qt::AlignCenter);
    personalBestLabel->setStyleSheet("font-size: 22px; color: #d32f2f; border: none;");
    layout->addWidget(personalBestLabel);
    
    // 显示全球最高分（从全球前十名中取第一名）
    int globalBestScore = globalTop10.isEmpty() ? 0 : globalTop10.first().score;
    QString globalBestPlayer = globalTop10.isEmpty() ? "" : globalTop10.first().username;
    QString globalText = globalBestPlayer.isEmpty() ? 
        QString("全球最高：<b style='color:#ff6f00;'>%1</b> 分").arg(QString::number(globalBestScore)) :
        QString("全球最高：<b style='color:#ff6f00;'>%1</b> 分 (%2)").arg(QString::number(globalBestScore), globalBestPlayer);
    QLabel* globalBestLabel = new QLabel(globalText);
    globalBestLabel->setAlignment(Qt::AlignCenter);
    globalBestLabel->setStyleSheet("font-size: 18px; color: #ff6f00; border: none;");
    layout->addWidget(globalBestLabel);// 居中显示卡片
    QGraphicsProxyWidget* proxy = m_scene->addWidget(card);
    proxy->setZValue(2001);
    QSize cardSize(350, 280);  // 增加高度以容纳新的标签
    card->setFixedSize(cardSize);
    proxy->setPos(sceneRect.center().x() - cardSize.width() / 2,
                  sceneRect.center().y() - cardSize.height() / 2);

    // 右下角按钮参数
    int btnDiameter = 70;
    int margin = 40;

    // 再来一次按钮（上）
    QPushButton* retryBtn = new QPushButton;
    retryBtn->setText("↻");
    retryBtn->setToolTip("再来一次");
    retryBtn->setFixedSize(btnDiameter, btnDiameter);
    retryBtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 35px;"
        "background: transparent;"
        "color: white;"
        "font-size: 32px;"
        "font-weight: bold;"
        "border: 3px solid #1565c0;"
        "}"
        "QPushButton:hover { background: #1565c0; }"
    );
    retryBtn->setCursor(Qt::PointingHandCursor);

    QGraphicsProxyWidget* retryProxy = m_scene->addWidget(retryBtn);    retryProxy->setZValue(2002);
    retryProxy->setPos(sceneRect.right() - btnDiameter - margin,
                       sceneRect.bottom() - btnDiameter * 3 - margin - 40);

    // 排行榜按钮（中）
    QPushButton* leaderboardBtn = new QPushButton;
    leaderboardBtn->setText("🏆");
    leaderboardBtn->setToolTip("查看排行榜");
    leaderboardBtn->setFixedSize(btnDiameter, btnDiameter);
    leaderboardBtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 35px;"
        "background: transparent;"
        "color: white;"
        "font-size: 28px;"
        "font-weight: bold;"
        "border: 3px solid #ff6f00;"
        "}"
        "QPushButton:hover { background: #ff6f00; }"
    );
    leaderboardBtn->setCursor(Qt::PointingHandCursor);

    QGraphicsProxyWidget* leaderboardProxy = m_scene->addWidget(leaderboardBtn);
    leaderboardProxy->setZValue(2002);
    leaderboardProxy->setPos(sceneRect.right() - btnDiameter - margin,
                            sceneRect.bottom() - btnDiameter * 2 - margin - 20);

    // 退出按钮（下）
    QPushButton* exitBtn = new QPushButton;
    exitBtn->setText("⌂");
    exitBtn->setToolTip("返回主界面");
    exitBtn->setFixedSize(btnDiameter, btnDiameter);
    exitBtn->setStyleSheet(
        "QPushButton {"
        "border-radius: 35px;"
        "background: transparent;"
        "color: white;"
        "font-size: 32px;"
        "font-weight: bold;"
        "border: 3px solid #b71c1c;"
        "}"
        "QPushButton:hover { background: #b71c1c; }"
    );
    exitBtn->setCursor(Qt::PointingHandCursor);

    QGraphicsProxyWidget* exitProxy = m_scene->addWidget(exitBtn);
    exitProxy->setZValue(2002);
    exitProxy->setPos(sceneRect.right() - btnDiameter - margin,
                      sceneRect.bottom() - btnDiameter - margin);
      // 排行榜按钮事件
    QObject::connect(leaderboardBtn, &QPushButton::clicked, [=]() {
        // 先移除游戏结束对话框
        m_scene->removeItem(proxy);
        m_scene->removeItem(retryProxy);
        m_scene->removeItem(leaderboardProxy);
        m_scene->removeItem(exitProxy);
        
        proxy->deleteLater();
        retryProxy->deleteLater();
        leaderboardProxy->deleteLater();
        exitProxy->deleteLater();
        
        // 显示排行榜，传递当前分数和回调函数
        showLeaderboard(score, onRetry, onExit);
    });
    
    // 按钮事件
    QObject::connect(retryBtn, &QPushButton::clicked, [=]() {
        // 先从场景中移除，但延迟删除避免在事件处理过程中删除对象
        m_scene->removeItem(proxy);
        m_scene->removeItem(retryProxy);
        m_scene->removeItem(leaderboardProxy);
        m_scene->removeItem(exitProxy);        // 使用 deleteLater 延迟删除，避免在事件处理过程中删除对象
        proxy->deleteLater();
        retryProxy->deleteLater();
        leaderboardProxy->deleteLater();
        exitProxy->deleteLater();

        if (onRetry) onRetry();
    });
    QObject::connect(exitBtn, &QPushButton::clicked, [=]() {
        // 先从场景中移除，但延迟删除避免在事件处理过程中删除对象
        m_scene->removeItem(proxy);
        m_scene->removeItem(retryProxy);
        m_scene->removeItem(leaderboardProxy);
        m_scene->removeItem(exitProxy);

        // 使用 deleteLater 延迟删除，避免在事件处理过程中删除对象
        proxy->deleteLater();
        retryProxy->deleteLater();
        leaderboardProxy->deleteLater();
        exitProxy->deleteLater();

        if (onExit) onExit();
    });
}

void UIManager::updateGlobalLeaderboard(const QString& username, int score)
{
    QString gameRecordPath = QCoreApplication::applicationDirPath() + "/game_record.ini";
    QSettings gameSettings(gameRecordPath, QSettings::IniFormat);
    
    // 获取当前时间戳，精确到小时
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm");
    
    // 读取现有的全球前十名记录
    QList<ScoreRecord> globalRecords;
    for (int i = 1; i <= 10; ++i) {
        QString scoreKey = QString("GlobalTop10/rank%1_score").arg(i);
        QString userKey = QString("GlobalTop10/rank%1_user").arg(i);
        QString timeKey = QString("GlobalTop10/rank%1_time").arg(i);
        
        if (gameSettings.contains(scoreKey)) {
            ScoreRecord record;
            record.score = gameSettings.value(scoreKey).toInt();
            record.username = gameSettings.value(userKey).toString();
            record.timestamp = gameSettings.value(timeKey).toString();
            globalRecords.append(record);
        }
    }
    
    // 添加新记录
    ScoreRecord newRecord;
    newRecord.username = username;
    newRecord.score = score;
    newRecord.timestamp = timestamp;
    globalRecords.append(newRecord);
    
    // 按分数降序排序
    std::sort(globalRecords.begin(), globalRecords.end(), 
              [](const ScoreRecord& a, const ScoreRecord& b) {
                  return a.score > b.score;
              });
    
    // 只保留前十名
    if (globalRecords.size() > 10) {
        globalRecords = globalRecords.mid(0, 10);
    }
    
    // 清除旧记录
    gameSettings.beginGroup("GlobalTop10");
    gameSettings.remove("");
    gameSettings.endGroup();
    
    // 保存新的前十名记录
    for (int i = 0; i < globalRecords.size(); ++i) {
        QString scoreKey = QString("GlobalTop10/rank%1_score").arg(i + 1);
        QString userKey = QString("GlobalTop10/rank%1_user").arg(i + 1);
        QString timeKey = QString("GlobalTop10/rank%1_time").arg(i + 1);
        
        gameSettings.setValue(scoreKey, globalRecords[i].score);
        gameSettings.setValue(userKey, globalRecords[i].username);
        gameSettings.setValue(timeKey, globalRecords[i].timestamp);
    }
    
    gameSettings.sync();
}

void UIManager::updatePersonalLeaderboard(const QString& username, int score)
{
    QString userDataPath = QCoreApplication::applicationDirPath() + "/user_data.ini";
    QSettings userSettings(userDataPath, QSettings::IniFormat);
    
    // 获取当前时间戳，精确到小时
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm");
    
    // 读取现有的个人前十名记录
    QList<ScoreRecord> personalRecords;
    for (int i = 1; i <= 10; ++i) {
        QString scoreKey = QString("Users/%1/top10/rank%2_score").arg(username).arg(i);
        QString timeKey = QString("Users/%1/top10/rank%2_time").arg(username).arg(i);
        
        if (userSettings.contains(scoreKey)) {
            ScoreRecord record;
            record.score = userSettings.value(scoreKey).toInt();
            record.username = username;
            record.timestamp = userSettings.value(timeKey).toString();
            personalRecords.append(record);
        }
    }
    
    // 添加新记录
    ScoreRecord newRecord;
    newRecord.username = username;
    newRecord.score = score;
    newRecord.timestamp = timestamp;
    personalRecords.append(newRecord);
    
    // 按分数降序排序
    std::sort(personalRecords.begin(), personalRecords.end(), 
              [](const ScoreRecord& a, const ScoreRecord& b) {
                  return a.score > b.score;
              });
    
    // 只保留前十名
    if (personalRecords.size() > 10) {
        personalRecords = personalRecords.mid(0, 10);
    }
    
    // 清除旧记录
    QString groupName = QString("Users/%1/top10").arg(username);
    userSettings.beginGroup(groupName);
    userSettings.remove("");
    userSettings.endGroup();
    
    // 保存新的前十名记录
    for (int i = 0; i < personalRecords.size(); ++i) {
        QString scoreKey = QString("Users/%1/top10/rank%2_score").arg(username).arg(i + 1);
        QString timeKey = QString("Users/%1/top10/rank%2_time").arg(username).arg(i + 1);
        
        userSettings.setValue(scoreKey, personalRecords[i].score);
        userSettings.setValue(timeKey, personalRecords[i].timestamp);
    }
    
    userSettings.sync();
}

QList<UIManager::ScoreRecord> UIManager::getGlobalTop10() const
{
    QString gameRecordPath = QCoreApplication::applicationDirPath() + "/game_record.ini";
    QSettings gameSettings(gameRecordPath, QSettings::IniFormat);
    
    QList<ScoreRecord> records;
    for (int i = 1; i <= 10; ++i) {
        QString scoreKey = QString("GlobalTop10/rank%1_score").arg(i);
        QString userKey = QString("GlobalTop10/rank%1_user").arg(i);
        QString timeKey = QString("GlobalTop10/rank%1_time").arg(i);
        
        if (gameSettings.contains(scoreKey)) {
            ScoreRecord record;
            record.score = gameSettings.value(scoreKey).toInt();
            record.username = gameSettings.value(userKey).toString();
            record.timestamp = gameSettings.value(timeKey).toString();
            records.append(record);
        }
    }
    
    return records;
}

QList<UIManager::ScoreRecord> UIManager::getPersonalTop10(const QString& username) const
{
    QString userDataPath = QCoreApplication::applicationDirPath() + "/user_data.ini";
    QSettings userSettings(userDataPath, QSettings::IniFormat);
    
    QList<ScoreRecord> records;
    for (int i = 1; i <= 10; ++i) {
        QString scoreKey = QString("Users/%1/top10/rank%2_score").arg(username).arg(i);
        QString timeKey = QString("Users/%1/top10/rank%2_time").arg(username).arg(i);
        
        if (userSettings.contains(scoreKey)) {
            ScoreRecord record;
            record.score = userSettings.value(scoreKey).toInt();
            record.username = username;
            record.timestamp = userSettings.value(timeKey).toString();
            records.append(record);
        }
    }
    
    return records;
}

void UIManager::showLeaderboard(int currentScore, const std::function<void()>& onRetry, const std::function<void()>& onExit)
{
    QGraphicsView* view = getView();
    if (!view) return;

    QRectF sceneRect = view->mapToScene(view->viewport()->rect()).boundingRect();
    QString currentUsername = getCurrentUsername();
    
    // 获取排行榜数据
    QList<ScoreRecord> globalTop10 = getGlobalTop10();
    QList<ScoreRecord> personalTop10 = getPersonalTop10(currentUsername);
    
    // 创建排行榜窗口
    QWidget* leaderboardWidget = new QWidget;
    leaderboardWidget->setStyleSheet("background: #f5f5f5; border: 2px solid #1976d2; border-radius: 10px;");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(leaderboardWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(25, 25, 25, 25);
    
    // 标题
    QLabel* titleLabel = new QLabel("🏆 排行榜 🏆");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #1976d2; border: none; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);
    
    // 创建标签页容器
    QWidget* tabContainer = new QWidget;
    QHBoxLayout* tabLayout = new QHBoxLayout(tabContainer);
    tabLayout->setSpacing(20);
    
    // 个人前十名
    QWidget* personalWidget = new QWidget;
    personalWidget->setStyleSheet("background: white; border: 1px solid #ccc; border-radius: 8px; padding: 10px;");
    QVBoxLayout* personalLayout = new QVBoxLayout(personalWidget);
    personalLayout->setSpacing(8);
    
    QLabel* personalTitle = new QLabel(QString("个人前十名 (%1)").arg(currentUsername));
    personalTitle->setAlignment(Qt::AlignCenter);
    personalTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #d32f2f; border: none; margin-bottom: 8px;");
    personalLayout->addWidget(personalTitle);
    
    if (personalTop10.isEmpty()) {
        QLabel* noRecordLabel = new QLabel("暂无记录");
        noRecordLabel->setAlignment(Qt::AlignCenter);
        noRecordLabel->setStyleSheet("color: #666; font-size: 14px;");
        personalLayout->addWidget(noRecordLabel);
    } else {
        for (int i = 0; i < personalTop10.size(); ++i) {
            const auto& record = personalTop10[i];
            QString rankText = QString("%1. %2 分 - %3")
                .arg(i + 1, 2, 10, QChar('0'))
                .arg(record.score)
                .arg(record.timestamp);
            
            QLabel* rankLabel = new QLabel(rankText);
            rankLabel->setStyleSheet(i == 0 ? 
                "font-size: 14px; color: #d32f2f; font-weight: bold; padding: 2px;" :
                "font-size: 12px; color: #333; padding: 2px;");
            personalLayout->addWidget(rankLabel);
        }
    }
    
    personalLayout->addStretch();
    tabLayout->addWidget(personalWidget);
    
    // 全球前十名
    QWidget* globalWidget = new QWidget;
    globalWidget->setStyleSheet("background: white; border: 1px solid #ccc; border-radius: 8px; padding: 10px;");
    QVBoxLayout* globalLayout = new QVBoxLayout(globalWidget);
    globalLayout->setSpacing(8);
    
    QLabel* globalTitle = new QLabel("全球前十名");
    globalTitle->setAlignment(Qt::AlignCenter);
    globalTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #ff6f00; border: none; margin-bottom: 8px;");
    globalLayout->addWidget(globalTitle);
    
    if (globalTop10.isEmpty()) {
        QLabel* noRecordLabel = new QLabel("暂无记录");
        noRecordLabel->setAlignment(Qt::AlignCenter);
        noRecordLabel->setStyleSheet("color: #666; font-size: 14px;");
        globalLayout->addWidget(noRecordLabel);
    } else {
        for (int i = 0; i < globalTop10.size(); ++i) {
            const auto& record = globalTop10[i];
            QString rankText = QString("%1. %2 分 - %3 (%4)")
                .arg(i + 1, 2, 10, QChar('0'))
                .arg(record.score)
                .arg(record.timestamp)
                .arg(record.username);
            
            QLabel* rankLabel = new QLabel(rankText);
            QString style = "font-size: 12px; padding: 2px;";
            if (i == 0) {
                style = "font-size: 14px; color: #ff6f00; font-weight: bold; padding: 2px;";
            } else if (record.username == currentUsername) {
                style = "font-size: 12px; color: #1976d2; font-weight: bold; padding: 2px;";
            } else {
                style = "font-size: 12px; color: #333; padding: 2px;";
            }
            rankLabel->setStyleSheet(style);
            globalLayout->addWidget(rankLabel);
        }
    }
    
    globalLayout->addStretch();
    tabLayout->addWidget(globalWidget);
    
    mainLayout->addWidget(tabContainer);
    
    // 关闭按钮
    QPushButton* closeBtn = new QPushButton("关闭");
    closeBtn->setFixedSize(80, 35);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "background: #1976d2;"
        "color: white;"
        "font-size: 14px;"
        "font-weight: bold;"
        "border: none;"
        "border-radius: 6px;"
        "}"
        "QPushButton:hover { background: #1565c0; }"
    );
    closeBtn->setCursor(Qt::PointingHandCursor);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeBtn);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
      // 显示排行榜窗口
    QGraphicsProxyWidget* leaderboardProxy = m_scene->addWidget(leaderboardWidget);
    leaderboardProxy->setZValue(2003);
    
    // 根据视口大小动态调整排行榜窗口尺寸
    QRect viewportRect = view->viewport()->rect();
    int windowWidth = qMin(800, static_cast<int>(viewportRect.width() * 0.9));  // 最大800px，或视口宽度的90%
    int windowHeight = qMin(600, static_cast<int>(viewportRect.height() * 0.85)); // 最大600px，或视口高度的85%
    QSize windowSize(windowWidth, windowHeight);
    
    leaderboardWidget->setFixedSize(windowSize);
    leaderboardProxy->setPos(sceneRect.center().x() - windowSize.width() / 2,
                            sceneRect.center().y() - windowSize.height() / 2);// 关闭按钮事件
    QObject::connect(closeBtn, &QPushButton::clicked, [=]() {
        // 移除排行榜窗口
        m_scene->removeItem(leaderboardProxy);
        leaderboardProxy->deleteLater();
        
        // 重新显示游戏结束对话框，但不更新排行榜数据
        showGameOverDialog(currentScore, onRetry, onExit, false);
    });
}

QGraphicsView* UIManager::getView() const
{
    if (!m_scene || m_scene->views().isEmpty()) {
        return nullptr;
    }
    return m_scene->views().first();
}

bool UIManager::isUIManagerObject(QGraphicsItem* item) const
{
    if (!item) {
        return false;
    }

    // 检查是否为暂停遮罩
    if (item == m_pauseOverlay) {
        return true;
    }

    // 检查是否为暂停文本
    if (item == m_pauseText) {
        return true;
    }
      // 检查是否为按钮的代理项 (QGraphicsProxyWidget)
    QGraphicsProxyWidget* proxyWidget = qgraphicsitem_cast<QGraphicsProxyWidget*>(item);
    if (proxyWidget) {
        QWidget* widget = proxyWidget->widget();
        if (widget == m_pauseButton ||
            widget == m_warningButton||
            widget == m_scoreLabel ||
            widget == m_scorePopupLabel) {
            return true;
        }
    }

    return false;
}

void UIManager::showScorePopup(int points, const QString& reason)
{
    if (!m_scorePopupLabel) return;

    // 停止现有计时器（如果仍在显示）
    m_popupTimer->stop();

    // 设置文本内容 - 使用HTML格式使reason部分更大并设为白色
    QString text = QString("<span style='color:white; font-size:35px;'>%1</span>       <span style='color:yellow;'>+%2</span>")
        .arg(reason).arg(points);
    m_scorePopupLabel->setText(text);

    // 将主分数标签颜色改为黄色
    if (m_scoreLabel) {
        m_scoreLabel->setStyleSheet("color: yellow; "
                                   "border: none; "
                                   "background: transparent; "
                                   "padding: 6px 18px;");
    }

    QGraphicsView* view = getView();
    if (!view) return;
    QRect vp = view->viewport()->rect();

    // 更新临时得分提示标签位置
    if (m_scorePopupLabel) {
        m_scorePopupLabel->setParent(view->viewport());
        // 放在分数标签下方居中位置
        m_scorePopupLabel->setGeometry(vp.width() - 400, 65, 350, 50);
    }

    // 显示标签
    m_scorePopupLabel->show();

    // 启动计时器，2秒后隐藏
    m_popupTimer->start(2000);

    // 更新UI确保位置正确
    updateUI();
}

void UIManager::resetUI()
{
    // 重置得分标签
    if (m_scoreLabel) {
        m_scoreLabel->setText("0");
        // 恢复为白色
        m_scoreLabel->setStyleSheet("color: white; "
                                   "border: none; "
                                   "background: transparent; "
                                   "padding: 6px 18px;");
    }

    // 重置得分倍率条
    if (m_scoreMultiplierContainer && m_scoreMultiplierBar && m_scoreMultiplierLabel) {
        // 设置进度条初始状态
        int padding = 2; // 内边距
        int barHeight = m_scoreMultiplierContainer->height() - (padding * 2);
        int minBarWidth = barHeight; // 保证最小是一个圆形

        // 设置初始宽度为最小圆形宽度，而不是1像素
        m_scoreMultiplierBar->setGeometry(padding, padding, minBarWidth, barHeight);

        // 设置圆角半径
        int borderRadius = qMin(barHeight / 2, 13);

        // 设置样式，确保有正确的颜色填充
        m_scoreMultiplierBar->setStyleSheet(
            QString("background-color: rgba(255, 215, 0, 150); "
                    "border: none; border-radius: %1px;").arg(borderRadius)
        );

        // 重置标签文字
        m_scoreMultiplierLabel->setText("x1.0");

        // 默认状态下隐藏(倍率为1.0时不显示)
        m_scoreMultiplierContainer->hide();
    }

    // 隐藏弹出标签
    if (m_scorePopupLabel) {
        m_scorePopupLabel->hide();
    }

    // 停止计时器
    if (m_popupTimer) {
        m_popupTimer->stop();
    }

    // 更新UI确保位置正确
    updateUI();
}

void UIManager::showScoreMultiplier(double multiplier) {
    if (!m_scoreMultiplierContainer || !m_scoreMultiplierBar || !m_scoreMultiplierLabel)
        return;

    // 确保倍率条在视口中显示
    QGraphicsView* view = getView();
    if (!view) return;
    m_scoreMultiplierContainer->setParent(view->viewport());

    // 如果倍率为1.0或小于1.0，直接隐藏倍率条并返回
    if (multiplier <= 1.0) {
        m_scoreMultiplierContainer->hide();
        return;
    }

    // 获取GameScene实例
    GameScene* gameScene = qobject_cast<GameScene*>(m_scene);
    // 获取最大倍率值
    double maxScoreMultiplier = gameScene->getMaxAward_Score() - 1.0;

    // 更新标签文字
    m_scoreMultiplierLabel->setText(QString("x%1").arg(multiplier, 0, 'f', 1));

    // 容器尺寸
    int containerWidth = m_scoreMultiplierContainer->width();
    int containerHeight = m_scoreMultiplierContainer->height();

    // 设置容器样式
    m_scoreMultiplierContainer->setStyleSheet(
        "background-color: rgba(50, 50, 50, 150);"
        "border: 1px solid white;"
        "border-radius: 15px;"
    );

    // 计算进度条比例
    double ratio = (multiplier - 1.0) / maxScoreMultiplier;
    if (ratio < 0) ratio = 0;
    if (ratio > 1) ratio = 1;

    // 适当的内边距
    int padding = 2; // 较小的内边距

    // 最终进度条尺寸和位置
    int barHeight = containerHeight - (padding * 2);

    // 计算宽度（最小为一个圆，最大为容器宽度减去内边距）
    int minBarWidth = barHeight; // 保证最小是一个圆形
    int maxBarWidth = containerWidth - (padding * 2);
    int barWidth = minBarWidth + static_cast<int>((maxBarWidth - minBarWidth) * ratio);

    // 确保不超出边界
    barWidth = qMin(maxBarWidth, barWidth);

    // 设置进度条位置和大小
    m_scoreMultiplierBar->setGeometry(padding, padding, barWidth, barHeight);

    // 确保圆角与容器的圆角协调
    // 使用随宽度调整的圆角半径
    int borderRadius = qMin(barHeight / 2, 13); // 使用较小值避免圆角过大

    // 为所有倍率设置统一的圆角处理方式
    QString baseStyle = QString("border: none; border-radius: %1px;").arg(borderRadius);

    // 根据倍率值设置不同的颜色样式
    if (multiplier >= 5.0) {
        // 金色渐变
        m_scoreMultiplierBar->setStyleSheet(
            "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0,"
            "stop:0 rgba(255, 215, 0, 200), stop:1 rgba(255, 165, 0, 200));" +
            baseStyle
        );
    } else if (multiplier >= 2.0) {
        // 橙色
        m_scoreMultiplierBar->setStyleSheet(
            "background-color: rgba(255, 165, 0, 180);" +
            baseStyle
        );
    } else {
        // 默认淡黄色
        m_scoreMultiplierBar->setStyleSheet(
            "background-color: rgba(255, 215, 0, 150);" +
            baseStyle
        );
    }

    // 显示倍率条
    m_scoreMultiplierContainer->show();
}

QString UIManager::getCurrentUsername() const
{
    // 从用户数据文件中获取当前登录的用户名
    QString userDataPath = QCoreApplication::applicationDirPath() + "/user_data.ini";
    QSettings userSettings(userDataPath, QSettings::IniFormat);
    
    // 尝试从RememberPassword功能中获取当前用户名
    QString username = userSettings.value("General/Username", "").toString();
    
    // 如果没有找到，则尝试从主窗口标题中解析
    if (username.isEmpty()) {
        QGraphicsView* view = getView();
        if (view) {
            QWidget* topLevelWidget = view->window();
            if (topLevelWidget) {
                QString windowTitle = topLevelWidget->windowTitle();
                // 窗口标题格式可能为: "滑雪大冒险 - 欢迎 用户名" 或 "滑雪大冒险 - 用户名"
                if (windowTitle.contains(" - 欢迎 ")) {
                    QStringList parts = windowTitle.split(" - 欢迎 ");
                    if (parts.size() == 2) {
                        username = parts[1];
                    }
                } else if (windowTitle.contains(" - ")) {
                    QStringList parts = windowTitle.split(" - ");
                    if (parts.size() == 2) {
                        username = parts[1];
                    }
                }
            }
        }
    }
    
    // 如果仍然没有找到用户名，使用默认值
    if (username.isEmpty()) {
        username = "Guest";
    }
    
    return username;
}

