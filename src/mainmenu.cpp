#include "mainmenu.h"
#include "mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <QFontDatabase>
#include <QPixmap>
#include <QIcon>
#include <QMessageBox>
#include <QDateTime>
#include <QLinearGradient>
#include <QPainter>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QRandomGenerator>

MainMenu::MainMenu(const QString& username, QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_username(username)
    , m_animationFrame(0)
    , m_gameWindow(nullptr)
{
    setWindowTitle("滑雪大冒险 - 主菜单");
    setWindowIcon(QIcon(":/resource/images/icons/gui_title.png"));
    setMinimumSize(1000, 700);
    resize(1200, 800);
    
    // 初始化设置
    QString settingsPath = QApplication::applicationDirPath() + "/user_data.ini";
    m_settings = new QSettings(settingsPath, QSettings::IniFormat);
    
    setupUI();
    setupStyles();
    setupAnimations();
    
    // 居中显示
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        QPoint center = screenGeometry.center() - QPoint(width() / 2, height() / 2);
        move(center);
    }
}

MainMenu::~MainMenu()
{
    delete m_settings;
    if (m_gameWindow) {
        delete m_gameWindow;
    }
}

void MainMenu::setupUI()
{
    m_centralWidget = new QWidget();
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setSpacing(30);
    m_mainLayout->setContentsMargins(50, 40, 50, 40);
    
    createBackground();
    createHeader();
    createMenuButtons();
}

void MainMenu::createBackground()
{
    // 创建背景
    m_backgroundLabel = new QLabel(m_centralWidget);
    m_backgroundLabel->setGeometry(0, 0, width(), height());
    m_backgroundLabel->lower(); // 置于底层
    
    // 创建渐变背景
    QPixmap backgroundPixmap(size());
    QPainter painter(&backgroundPixmap);
    
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, QColor(135, 206, 235, 200)); // 天空蓝
    gradient.setColorAt(0.3, QColor(176, 224, 230, 180)); // 浅蓝
    gradient.setColorAt(0.7, QColor(255, 255, 255, 160)); // 白色（雪地）
    gradient.setColorAt(1, QColor(240, 248, 255, 180)); // 雪白
    
    painter.fillRect(backgroundPixmap.rect(), gradient);
      // 添加雪花装饰
    painter.setPen(QPen(QColor(255, 255, 255, 100), 2));
    painter.setBrush(QBrush(QColor(255, 255, 255, 120)));
    
    for (int i = 0; i < 50; ++i) {
        int x = QRandomGenerator::global()->bounded(width());
        int y = QRandomGenerator::global()->bounded(height());
        int size = QRandomGenerator::global()->bounded(8) + 2;
        painter.drawEllipse(x, y, size, size);
    }
    
    m_backgroundLabel->setPixmap(backgroundPixmap);
    
    // 设置背景动画定时器
    m_backgroundTimer = new QTimer(this);
    connect(m_backgroundTimer, &QTimer::timeout, this, &MainMenu::updateBackgroundAnimation);
    m_backgroundTimer->start(5000); // 每5秒更新一次背景
}

void MainMenu::createHeader()
{
    // 标题区域
    QFrame* headerFrame = new QFrame();
    headerFrame->setObjectName("headerFrame");
    m_mainLayout->addWidget(headerFrame);
    
    QVBoxLayout* headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setSpacing(15);
    headerLayout->setAlignment(Qt::AlignCenter);
    
    // 主标题
    m_titleLabel = new QLabel("❄️ 滑雪大冒险 ⛷️");
    m_titleLabel->setObjectName("titleLabel");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(m_titleLabel);
    
    // 副标题
    m_subtitleLabel = new QLabel("极限滑雪 · 无尽冒险");
    m_subtitleLabel->setObjectName("subtitleLabel");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(m_subtitleLabel);
    
    // 欢迎标签
    m_welcomeLabel = new QLabel(QString("欢迎回来，%1！").arg(m_username));
    m_welcomeLabel->setObjectName("welcomeLabel");
    m_welcomeLabel->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(m_welcomeLabel);
    
    // 添加阴影效果
    QGraphicsDropShadowEffect* titleShadow = new QGraphicsDropShadowEffect();
    titleShadow->setBlurRadius(15);
    titleShadow->setColor(QColor(0, 0, 0, 120));
    titleShadow->setOffset(2, 2);
    m_titleLabel->setGraphicsEffect(titleShadow);
}

void MainMenu::createMenuButtons()
{
    // 菜单区域
    m_menuFrame = new QFrame();
    m_menuFrame->setObjectName("menuFrame");
    m_menuFrame->setMaximumWidth(400);
    m_menuFrame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    
    m_menuLayout = new QVBoxLayout(m_menuFrame);
    m_menuLayout->setSpacing(20);
    m_menuLayout->setContentsMargins(40, 30, 40, 30);
    
    // 开始游戏按钮
    m_startGameButton = new QPushButton("🎮 开始游戏");
    m_startGameButton->setObjectName("startGameButton");
    connect(m_startGameButton, &QPushButton::clicked, this, &MainMenu::onStartGameClicked);
    addButtonHoverEffect(m_startGameButton);
    m_menuLayout->addWidget(m_startGameButton);
    
    // 排行榜按钮
    m_leaderboardButton = new QPushButton("🏆 排行榜");
    m_leaderboardButton->setObjectName("leaderboardButton");
    connect(m_leaderboardButton, &QPushButton::clicked, this, &MainMenu::onLeaderboardClicked);
    addButtonHoverEffect(m_leaderboardButton);
    m_menuLayout->addWidget(m_leaderboardButton);
    
    // 游戏说明按钮
    m_instructionsButton = new QPushButton("📖 游戏说明");
    m_instructionsButton->setObjectName("instructionsButton");
    connect(m_instructionsButton, &QPushButton::clicked, this, &MainMenu::onInstructionsClicked);
    addButtonHoverEffect(m_instructionsButton);
    m_menuLayout->addWidget(m_instructionsButton);
    
    // 退出游戏按钮
    m_exitButton = new QPushButton("🚪 退出游戏");
    m_exitButton->setObjectName("exitButton");
    connect(m_exitButton, &QPushButton::clicked, this, &MainMenu::onExitClicked);
    addButtonHoverEffect(m_exitButton);
    m_menuLayout->addWidget(m_exitButton);
    
    // 将菜单框居中添加到主布局
    QHBoxLayout* centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(m_menuFrame);
    centerLayout->addStretch();
    
    m_mainLayout->addLayout(centerLayout);
    m_mainLayout->addStretch();
}

void MainMenu::setupStyles()
{
    // 加载自定义字体（如果有的话）
    int fontId = QFontDatabase::addApplicationFont(":/resource/fonts/1.ttf");
    QString fontFamily = "Arial"; // 默认字体
    if (fontId >= 0) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.empty()) {
            fontFamily = fontFamilies.at(0);
        }
    }
    
    QString styleSheet = QString(R"(
        /* 主窗口背景 */
        QMainWindow {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #87CEEB, stop:0.3 #B0E0E6,
                stop:0.7 #FFFFFF, stop:1 #F0F8FF);
        }

        /* 标题样式 */
        #titleLabel {
            font-family: "%1";
            font-size: 48px;
            font-weight: bold;
            color: #2C3E50;
            margin: 20px 0;
        }

        #subtitleLabel {
            font-family: "%1";
            font-size: 18px;
            color: #34495E;
            font-style: italic;
            margin-bottom: 10px;
        }

        #welcomeLabel {
            font-family: "%1";
            font-size: 16px;
            color: #7F8C8D;
            margin-bottom: 20px;
        }

        /* 头部区域 */
        #headerFrame {
            background: rgba(255, 255, 255, 80);
            border-radius: 20px;
            border: 2px solid rgba(255, 255, 255, 150);
            margin-bottom: 30px;
            padding: 20px;
        }

        /* 菜单框架 */
        #menuFrame {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(255, 255, 255, 200),
                stop:1 rgba(240, 248, 255, 180));
            border-radius: 25px;
            border: 3px solid rgba(70, 130, 180, 100);

        }

        /* 菜单按钮基础样式 */
        QPushButton {
            font-family: "%1";
            font-size: 18px;
            font-weight: bold;
            color: white;
            border: none;
            border-radius: 15px;
            padding: 15px 25px;
            margin: 5px 0;
            min-height: 20px;
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3498DB, stop:1 #2980B9);
        }

        /* 开始游戏按钮 */
        #startGameButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #E74C3C, stop:1 #C0392B);
            font-size: 20px;
        }

        #startGameButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #EC7063, stop:1 #E74C3C);

        }

        /* 排行榜按钮 */
        #leaderboardButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #F39C12, stop:1 #E67E22);
        }

        #leaderboardButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #F7DC6F, stop:1 #F39C12);
        }

        /* 游戏说明按钮 */
        #instructionsButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #27AE60, stop:1 #229954);
        }

        #instructionsButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #58D68D, stop:1 #27AE60);
        }

        /* 退出游戏按钮 */
        #exitButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #95A5A6, stop:1 #7F8C8D);
        }

        #exitButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #BDC3C7, stop:1 #95A5A6);
        }

        /* 按钮按下效果 */
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #34495E, stop:1 #2C3E50);
        }
    )").arg(fontFamily);
    
    setStyleSheet(styleSheet);
}

void MainMenu::setupAnimations()
{
    // 标题动画
    m_titleAnimation = new QPropertyAnimation(m_titleLabel, "geometry");
    m_titleAnimation->setDuration(2000);
    m_titleAnimation->setEasingCurve(QEasingCurve::OutBounce);
}

void MainMenu::addButtonHoverEffect(QPushButton* button)
{
    // 添加阴影效果
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 100));
    shadow->setOffset(0, 5);
    button->setGraphicsEffect(shadow);
    
    // 悬停效果现在通过CSS样式表处理，不需要信号连接
}

void MainMenu::onStartGameClicked()
{
    // 隐藏主菜单
    hide();
    
    // 创建并显示游戏窗口
    if (!m_gameWindow) {
        m_gameWindow = new MainWindow();
        m_gameWindow->setWindowTitle(QString("滑雪大冒险 - %1").arg(m_username));
        
        // 当游戏窗口关闭时，重新显示主菜单
        connect(m_gameWindow, &QMainWindow::destroyed, this, [this]() {
            m_gameWindow = nullptr;
            show();
        });
    }
    
    m_gameWindow->show();
    m_gameWindow->raise();
    m_gameWindow->activateWindow();
}

void MainMenu::onLeaderboardClicked()
{
    showLeaderboardDialog();
}

void MainMenu::onInstructionsClicked()
{
    showInstructionsDialog();
}

void MainMenu::onExitClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "退出游戏", 
        "确定要退出游戏吗？",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        QApplication::quit();
    }
}

void MainMenu::onButtonHovered()
{
    // 按钮悬停效果可以在这里添加额外的动画
}

void MainMenu::onButtonLeft()
{
    // 按钮离开悬停状态的效果
}

void MainMenu::updateBackgroundAnimation()
{
    m_animationFrame = (m_animationFrame + 1) % 4;
    // 这里可以添加背景动画效果，比如雪花飘落
}

void MainMenu::showLeaderboardDialog()
{
    LeaderboardDialog dialog(this);
    dialog.exec();
}

void MainMenu::showInstructionsDialog()
{
    InstructionsDialog dialog(this);
    dialog.exec();
}

void MainMenu::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    if (m_backgroundLabel) {
        m_backgroundLabel->setGeometry(0, 0, width(), height());
        createBackground(); // 重新创建背景以适应新尺寸
    }
}

// 排行榜对话框实现
LeaderboardDialog::LeaderboardDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("排行榜");
    setWindowIcon(QIcon(":/resource/images/icons/gui_title.png"));
    setModal(true);
    setFixedSize(500, 600);
    
    QString settingsPath = QApplication::applicationDirPath() + "/user_data.ini";
    m_settings = new QSettings(settingsPath, QSettings::IniFormat);
    
    setupUI();
    loadLeaderboardData();
    
    // 居中显示
    if (parent) {
        QPoint parentCenter = parent->geometry().center();
        move(parentCenter - QPoint(width() / 2, height() / 2));
    }
}

void LeaderboardDialog::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setSpacing(20);
    m_layout->setContentsMargins(30, 30, 30, 30);
    
    // 标题
    QLabel* titleLabel = new QLabel("🏆 排行榜");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(R"(
        font-size: 24px;
        font-weight: bold;
        color: #E67E22;
        margin-bottom: 20px;
    )");
    m_layout->addWidget(titleLabel);
    
    // 排行榜列表
    m_leaderboardList = new QListWidget();
    m_leaderboardList->setStyleSheet(R"(
        QListWidget {
            background: rgba(255, 255, 255, 200);
            border: 2px solid #BDC3C7;
            border-radius: 10px;
            font-size: 14px;
            padding: 10px;
        }
        QListWidget::item {
            padding: 8px;
            border-bottom: 1px solid #ECF0F1;
        }
        QListWidget::item:selected {
            background: #3498DB;
            color: white;
        }
    )");
    m_layout->addWidget(m_leaderboardList);
    
    // 关闭按钮
    m_closeButton = new QPushButton("关闭");
    m_closeButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #95A5A6, stop:1 #7F8C8D);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #BDC3C7, stop:1 #95A5A6);
        }
    )");
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeButton);
    buttonLayout->addStretch();
    
    m_layout->addLayout(buttonLayout);
}

void LeaderboardDialog::loadLeaderboardData()
{
    m_leaderboardList->clear();
    
    // 添加标签页切换按钮
    QWidget* tabWidget = new QWidget();
    QHBoxLayout* tabLayout = new QHBoxLayout(tabWidget);
    
    QPushButton* globalTabBtn = new QPushButton("全球排行榜");
    QPushButton* personalTabBtn = new QPushButton("个人记录");
    
    globalTabBtn->setStyleSheet(R"(
        QPushButton {
            background: #3498DB;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background: #2980B9;
        }
    )");
    
    personalTabBtn->setStyleSheet(R"(
        QPushButton {
            background: #E74C3C;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background: #C0392B;
        }
    )");
    
    tabLayout->addWidget(globalTabBtn);
    tabLayout->addWidget(personalTabBtn);
    m_layout->insertWidget(1, tabWidget);
    
    // 读取全球排行榜数据
    auto loadGlobalLeaderboard = [this]() {
        m_leaderboardList->clear();
        
        QString gameRecordPath = QApplication::applicationDirPath() + "/game_record.ini";
        QSettings gameSettings(gameRecordPath, QSettings::IniFormat);
        
        QList<QPair<QString, QPair<int, QString>>> globalScores; // username, (score, timestamp)
        
        for (int i = 1; i <= 10; ++i) {
            QString scoreKey = QString("GlobalTop10/rank%1_score").arg(i);
            QString userKey = QString("GlobalTop10/rank%1_user").arg(i);
            QString timeKey = QString("GlobalTop10/rank%1_time").arg(i);
            
            if (gameSettings.contains(scoreKey)) {
                QString username = gameSettings.value(userKey).toString();
                int score = gameSettings.value(scoreKey).toInt();
                QString timestamp = gameSettings.value(timeKey).toString();
                globalScores.append(qMakePair(username, qMakePair(score, timestamp)));
            }
        }
        
        if (globalScores.isEmpty()) {
            QListWidgetItem* item = new QListWidgetItem("暂无全球排行榜数据");
            item->setTextAlignment(Qt::AlignCenter);
            m_leaderboardList->addItem(item);
        } else {
            for (int i = 0; i < globalScores.size(); ++i) {
                QString rank;
                if (i == 0) rank = "🥇";
                else if (i == 1) rank = "🥈";
                else if (i == 2) rank = "🥉";
                else rank = QString("%1.").arg(i + 1);
                
                QString itemText = QString("%1 %2 - %3 分 (%4)")
                                  .arg(rank)
                                  .arg(globalScores[i].first)
                                  .arg(globalScores[i].second.first)
                                  .arg(globalScores[i].second.second);
                
                QListWidgetItem* item = new QListWidgetItem(itemText);
                if (i < 3) {
                    item->setFont(QFont("Arial", 12, QFont::Bold));
                }
                m_leaderboardList->addItem(item);
            }
        }
    };
    
    // 读取个人排行榜数据
    auto loadPersonalLeaderboard = [this]() {
        m_leaderboardList->clear();
        
        QString userDataPath = QApplication::applicationDirPath() + "/user_data.ini";
        QSettings userSettings(userDataPath, QSettings::IniFormat);
        
        // 获取当前用户名 (从主菜单获取)
        QString currentUsername = "";
        if (MainMenu* mainMenu = qobject_cast<MainMenu*>(parent())) {
            currentUsername = mainMenu->getUsername();
        }
        
        if (currentUsername.isEmpty()) {
            QListWidgetItem* item = new QListWidgetItem("无法获取用户信息");
            item->setTextAlignment(Qt::AlignCenter);
            m_leaderboardList->addItem(item);
            return;
        }
        
        QList<QPair<int, QString>> personalScores; // score, timestamp
        
        for (int i = 1; i <= 10; ++i) {
            QString scoreKey = QString("Users/%1/top10/rank%2_score").arg(currentUsername).arg(i);
            QString timeKey = QString("Users/%1/top10/rank%2_time").arg(currentUsername).arg(i);
            
            if (userSettings.contains(scoreKey)) {
                int score = userSettings.value(scoreKey).toInt();
                QString timestamp = userSettings.value(timeKey).toString();
                personalScores.append(qMakePair(score, timestamp));
            }
        }
        
        if (personalScores.isEmpty()) {
            QListWidgetItem* item = new QListWidgetItem(QString("暂无 %1 的个人记录").arg(currentUsername));
            item->setTextAlignment(Qt::AlignCenter);
            m_leaderboardList->addItem(item);
        } else {
            for (int i = 0; i < personalScores.size(); ++i) {
                QString rank;
                if (i == 0) rank = "🥇";
                else if (i == 1) rank = "🥈";
                else if (i == 2) rank = "🥉";
                else rank = QString("%1.").arg(i + 1);
                
                QString itemText = QString("%1 %2 分 - %3")
                                  .arg(rank)
                                  .arg(personalScores[i].first)
                                  .arg(personalScores[i].second);
                
                QListWidgetItem* item = new QListWidgetItem(itemText);
                if (i < 3) {
                    item->setFont(QFont("Arial", 12, QFont::Bold));
                }
                m_leaderboardList->addItem(item);
            }
        }
    };
    
    // 连接按钮事件
    connect(globalTabBtn, &QPushButton::clicked, loadGlobalLeaderboard);
    connect(personalTabBtn, &QPushButton::clicked, loadPersonalLeaderboard);
    
    // 默认显示全球排行榜
    loadGlobalLeaderboard();
}

// 游戏说明对话框实现
InstructionsDialog::InstructionsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("游戏说明");
    setWindowIcon(QIcon(":/resource/images/icons/gui_title.png"));
    setModal(true);
    setFixedSize(600, 700);
    
    setupUI();
    
    // 居中显示
    if (parent) {
        QPoint parentCenter = parent->geometry().center();
        move(parentCenter - QPoint(width() / 2, height() / 2));
    }
}

void InstructionsDialog::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setSpacing(20);
    m_layout->setContentsMargins(30, 30, 30, 30);
    
    // 标题
    QLabel* titleLabel = new QLabel("📖 游戏说明");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(R"(
        font-size: 24px;
        font-weight: bold;
        color: #27AE60;
        margin-bottom: 20px;
    )");
    m_layout->addWidget(titleLabel);
    
    // 游戏说明文本
    m_instructionsText = new QTextBrowser();
    m_instructionsText->setStyleSheet(R"(
        QTextBrowser {
            background: rgba(255, 255, 255, 200);
            border: 2px solid #BDC3C7;
            border-radius: 10px;
            font-size: 14px;
            padding: 15px;
            line-height: 1.6;
        }
    )");
    
    QString instructions = R"(
    <h2 style="color: #2C3E50;">🎮 游戏操作</h2>
    <ul>
        <li><b>A键 / 左箭头</b>：向左移动</li>
        <li><b>D键 / 右箭头</b>：向右移动</li>
        <li><b>空格键</b>：跳跃 / 空翻</li>
        <li><b>S键</b>：拾取/丢弃NPC</li>
    </ul>
    
    <h2 style="color: #2C3E50;">🎯 游戏目标</h2>
    <p>在雪山上滑雪，躲避雪崩，收集NPC伙伴，完成各种空翻动作来获得高分！</p>
    
    <h2 style="color: #2C3E50;">🏔️ 游戏机制</h2>
    <ul>
        <li><b>雪崩追击</b>：雪崩会持续追击玩家，不要被追上！</li>
        <li><b>NPC伙伴</b>：拾取企鹅和雪怪可以获得能力加成</li>
        <li><b>空翻得分</b>：完成360度空翻可以获得额外分数</li>
        <li><b>地形挑战</b>：利用地形起伏进行跳跃和空翻</li>
        <li><b>石头障碍</b>：小心避开石头，撞击会摔倒</li>
    </ul>
    
    <h2 style="color: #2C3E50;">🐧 NPC系统</h2>
    <ul>
        <li><b>企鹅</b>：提供速度加成和空翻能力增强</li>
        <li><b>雪怪形态1</b>：提供大幅速度加成，但无法空翻</li>
        <li><b>雪怪形态2</b>：平衡的能力加成，并可携带企鹅</li>
        <li><b>组合效果</b>：雪怪形态2 + 企鹅 = 最强组合！</li>
    </ul>
    
    <h2 style="color: #2C3E50;">⚡ 特殊技巧</h2>
    <ul>
        <li><b>空翻加速</b>：完成空翻后获得短时间速度提升</li>
        <li><b>伤害抵抗</b>：在加速状态下可以抵抗石头伤害</li>
        <li><b>企鹅护盾</b>：携带的企鹅可以抵抗一次摔倒</li>
        <li><b>得分倍率</b>：连续高难度动作可以提升得分倍率</li>
    </ul>
    
    <h2 style="color: #E74C3C;">⚠️ 注意事项</h2>
    <ul>
        <li>摔倒会重置所有得分倍率</li>
        <li>雪怪形态切换需要触发特定条件</li>
        <li>NPC拾取有冷却时间</li>
        <li>合理利用地形是获得高分的关键</li>
    </ul>
    
    <p style="text-align: center; color: #7F8C8D; margin-top: 20px;">
        <i>祝你在雪山上玩得愉快！🏔️❄️</i>
    </p>
    )";
    
    m_instructionsText->setHtml(instructions);
    m_layout->addWidget(m_instructionsText);
    
    // 关闭按钮
    m_closeButton = new QPushButton("我知道了");
    m_closeButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #27AE60, stop:1 #229954);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #58D68D, stop:1 #27AE60);
        }
    )");
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeButton);
    buttonLayout->addStretch();
    
    m_layout->addLayout(buttonLayout);
}
