#pragma once

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <functional>

class UIManager : public QObject
{
    Q_OBJECT

public:
    explicit UIManager(QGraphicsScene* scene, QObject* parent = nullptr);
    ~UIManager();

    // 初始化和清理
    void initialize();
    void cleanup();
    
    // UI管理方法
    void updateUI();
    void showPauseOverlay(bool show, int score = 0);
    void showWarningIndicator(bool show, qreal distance);
    void showGameOverDialog(int score, const std::function<void()>& onRetry, const std::function<void()>& onExit);
    void showNPCPickupCooldown(bool show, qreal progress = 0.0);
    void showFallRecovery(bool show, qreal progress = 0.0);
    
    // 设置与获取
    void setScore(int score);
    QPushButton* getPauseButton() const { return m_pauseButton; }
    QPushButton* getWarningButton() const { return m_warningButton; }
    bool isPauseTextVisible() const { return m_pauseText && m_pauseText->isVisible(); }
    QGraphicsRectItem* getPauseOverlay() const { return m_pauseOverlay; }
    QGraphicsTextItem* getPauseText() const { return m_pauseText; }
    
    // 判断物体是否为UI管理器管理的对象
    bool isUIManagerObject(QGraphicsItem* item) const;

    // 重置UI状态，用于游戏重新开始
    void resetUI();

public slots:
    // 显示得分弹出提示（槽函数）
    void showScorePopup(int points, const QString& reason);

signals:
    void pauseToggled(); // 暂停状态改变信号

private:
    // 场景引用
    QGraphicsScene* m_scene;
    
    // UI元素
    QGraphicsTextItem* m_pauseText;
    QPushButton* m_pauseButton;
    QGraphicsRectItem* m_pauseOverlay;
    QPushButton* m_warningButton;
    
    // NPC拾取冷却进度条
    QWidget* m_npcCooldownContainer;
    QWidget* m_npcCooldownProgress;

    // 摔倒恢复进度条
    QWidget* m_fallRecoveryContainer;
    QWidget* m_fallRecoveryProgress;
    
    // 得分相关
    QLabel* m_scoreLabel;
    QLabel* m_scorePopupLabel;
    QTimer* m_popupTimer;

    // 创建UI元素的辅助方法
    void createPauseElements();
    void createWarningElements();
    void createScoreLabel();
    void createNPCCooldownElements();
    void createFallRecoveryElements();
    void setupButtonStyle(QPushButton* button, const QString& iconPath, bool transparent = true);

    // 获取主视图
    QGraphicsView* getView() const;
};
