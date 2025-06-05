#pragma once

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QLabel>
#include <QPushButton>

class UIManager : public QObject
{
    Q_OBJECT

public:
    explicit UIManager(QGraphicsScene* scene, QObject* parent = nullptr);
    ~UIManager();

    // 初始化和清理
    void initialize();
    void cleanup();    // UI管理方法
    void updateUI();
    void showPauseOverlay(bool show, int score = 0);    void showWarningIndicator(bool show, qreal distance);
    void showGameOverDialog(int score, const std::function<void()>& onRetry, const std::function<void()>& onExit, bool updateLeaderboard = true);
    void showLeaderboard(); // 新增：显示排行榜
    void showNPCPickupCooldown(bool show, qreal progress = 0.0);
    void showFallRecovery(bool show, qreal progress = 0.0);
    void showFlipBoost(bool show, qreal progress = 0.0);
    // 新增：显示得分倍率条
    void showScoreMultiplier(double multiplier);
    // 设置与获取
    void createPauseElements();
    void setScore(int score);
    QPushButton* getPauseButton() const { return m_pauseButton; }
    QPushButton* getWarningButton() const { return m_warningButton; }
    bool isPauseTextVisible() const { return m_pauseText && m_pauseText->isVisible(); }
    QGraphicsRectItem* getPauseOverlay() const { return m_pauseOverlay; }
    QGraphicsTextItem* getPauseText() const { return m_pauseText; }
    
    // 判断物体是否为UI管理器管理的对象
    bool isUIManagerObject(QGraphicsItem* item) const;    // 重置UI状态，用于游戏重新开始
    void resetUI();

    // 获取当前登录用户名
    QString getCurrentUsername() const;

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
    
    // 空翻加速进度条
    QWidget* m_flipBoostContainer;
    QWidget* m_flipBoostProgress;

    // 得分倍率显示
    QWidget* m_scoreMultiplierContainer;
    QWidget* m_scoreMultiplierBar;
    QLabel* m_scoreMultiplierLabel;
    // 创建UI元素的辅助方法
    QLabel* m_scoreLabel;
    QLabel* m_scorePopupLabel;
    QTimer* m_popupTimer;

    // 创建UI元素的辅助方法
    void createWarningElements();
    void createScoreLabel();
    void createNPCCooldownElements();
    void createFallRecoveryElements();
    void createFlipBoostElements();
    void createScoreMultiplierElements(); // 新增：创建得分倍率条
    void setupButtonStyle(QPushButton* button, const QString& iconPath, bool transparent = true);    // 获取主视图
    QGraphicsView* getView() const;
    
    // 排行榜相关方法
    struct ScoreRecord {
        QString username;
        int score;
        QString timestamp;
    };
      void updateGlobalLeaderboard(const QString& username, int score);
    void updatePersonalLeaderboard(const QString& username, int score);
    QList<ScoreRecord> getGlobalTop10() const;
    QList<ScoreRecord> getPersonalTop10(const QString& username) const;
    void showLeaderboard(int currentScore, const std::function<void()>& onRetry, const std::function<void()>& onExit);
};
