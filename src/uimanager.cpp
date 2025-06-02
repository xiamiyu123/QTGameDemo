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
#include <qfontdatabase.h>
#include <QRadialGradient>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QTimer>

UIManager::UIManager(QGraphicsScene* scene, QObject* parent)
    : QObject(parent)
    , m_scene(scene)
    , m_pauseText(nullptr)
    , m_pauseButton(nullptr)
    , m_pauseOverlay(nullptr)    , m_warningButton(nullptr)
    , m_npcCooldownContainer(nullptr)
    , m_npcCooldownProgress(nullptr)
    , m_fallRecoveryContainer(nullptr)
    , m_fallRecoveryProgress(nullptr)
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
    }      // 场景会自动处理其中的项目
    m_pauseText = nullptr;
    m_pauseOverlay = nullptr;
    m_npcCooldownProgress = nullptr;
    m_fallRecoveryProgress = nullptr;
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

void UIManager::createScoreLabel()
{

    // 引入新字体
    int id = QFontDatabase::addApplicationFont(":/resource/fonts/Kalmansk-Regular.otf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);

    // 创建分数标签
    QFont scoreFont(family, 52, QFont::Bold);
    m_scoreLabel = new QLabel("0");
    m_scoreLabel->setFont(scoreFont);
    m_scoreLabel->setStyleSheet("color: yellow; "
                                "border: none; "
                                "background: transparent; "
                                "padding: 6px 18px;");
    m_scoreLabel->setAlignment(Qt::AlignRight);

    // 添加临时得分提示标签
    QFont popupFont(family, 30, QFont::Bold);
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
    });
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
      // 更新NPC冷却进度条位置（下移）
    if (m_npcCooldownContainer) {
        m_npcCooldownContainer->setParent(view->viewport());
        QRect vp = view->viewport()->rect();
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
        QRect vp = view->viewport()->rect();
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

    // 更新分数标签位置
    if (m_scoreLabel) {
        m_scoreLabel->setParent(view->viewport());
        QRect vp = view->viewport()->rect();
        m_scoreLabel->setGeometry(vp.width() - 250, -23, 200, 82);
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


void UIManager::showGameOverDialog(int score, const std::function<void()>& onRetry, const std::function<void()>& onExit)
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
    layout->addWidget(title);

    QLabel* scoreLabel = new QLabel(QString("本次得分：<b style='color:#1976d2;'>%1</b> 分").arg(QString::number(score)));
    scoreLabel->setAlignment(Qt::AlignCenter);
    scoreLabel->setStyleSheet("font-size: 26px; color: #1565c0; border: none;");
    layout->addWidget(scoreLabel);

    QString iniPath = QCoreApplication::applicationDirPath() + "/game_record.ini";
    QSettings settings(iniPath, QSettings::IniFormat);
    int bestScore = settings.value("General/bestScore", 0).toInt();
    if (score >= bestScore) {
        settings.setValue("General/bestScore", score);
        bestScore = score;
    }
    QLabel* bestLabel = new QLabel(QString("历史最高：<b style='color:#d32f2f;'>%1</b> 分").arg(QString::number(bestScore)));
    bestLabel->setAlignment(Qt::AlignCenter);
    bestLabel->setStyleSheet("font-size: 22px; color: #d32f2f; border: none;");
    layout->addWidget(bestLabel);

    // 居中显示卡片
    QGraphicsProxyWidget* proxy = m_scene->addWidget(card);
    proxy->setZValue(2001);
    QSize cardSize(350, 220);
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

    QGraphicsProxyWidget* retryProxy = m_scene->addWidget(retryBtn);
    retryProxy->setZValue(2002);
    retryProxy->setPos(sceneRect.right() - btnDiameter - margin,
                       sceneRect.bottom() - btnDiameter * 2 - margin - 20);

    // 退出按钮（下）
    QPushButton* exitBtn = new QPushButton;
    exitBtn->setText("⏻");
    exitBtn->setToolTip("退出游戏");
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
                      sceneRect.bottom() - btnDiameter - margin);    // 按钮事件
    QObject::connect(retryBtn, &QPushButton::clicked, [=]() {
        // 先从场景中移除，但延迟删除避免在事件处理过程中删除对象
        m_scene->removeItem(proxy);
        m_scene->removeItem(retryProxy);
        m_scene->removeItem(exitProxy);

        // 使用 deleteLater 延迟删除，避免在事件处理过程中删除对象
        proxy->deleteLater();
        retryProxy->deleteLater();
        exitProxy->deleteLater();

        if (onRetry) onRetry();
    });    QObject::connect(exitBtn, &QPushButton::clicked, [=]() {
        // 先从场景中移除，但延迟删除避免在事件处理过程中删除对象
        m_scene->removeItem(proxy);
        m_scene->removeItem(retryProxy);
        m_scene->removeItem(exitProxy);

        // 使用 deleteLater 延迟删除，避免在事件处理过程中删除对象
        proxy->deleteLater();
        retryProxy->deleteLater();
        exitProxy->deleteLater();

        if (onExit) onExit();
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
        if (widget == m_pauseButton || widget == m_warningButton || widget == m_npcCooldownContainer) {
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

    // 停止现有计时器（如果正在显示）
    m_popupTimer->stop();

    // 设置文本内容
    QString text = QString("%1    +%2").arg(reason).arg(points);
    m_scorePopupLabel->setText(text);

    // 显示标签
    m_scorePopupLabel->show();

    QGraphicsView* view = getView();
    if (!view) return;
    QRect vp = view->viewport()->rect();


    // 更新临时得分提示标签位置
    if (m_scorePopupLabel) {
        m_scorePopupLabel->setParent(view->viewport());
        // 放在分数标签下方居中位置
        m_scorePopupLabel->setGeometry(vp.width() - 350, 60, 300, 50);
    }


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