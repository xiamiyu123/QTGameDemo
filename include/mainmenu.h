#pragma once

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QEasingCurve>
#include <QTimer>
#include <QSettings>
#include <QDialog>
#include <QTextBrowser>
#include <QListWidget>
#include <QListWidgetItem>

class MainWindow; // 前向声明游戏主窗口

class MainMenu : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainMenu(const QString& username, QWidget *parent = nullptr);
    ~MainMenu();
    
    // 获取当前用户名
    QString getUsername() const { return m_username; }

private slots:
    void onStartGameClicked();
    void onLeaderboardClicked();
    void onInstructionsClicked();
    void onExitClicked();
    void onButtonHovered();
    void onButtonLeft();
    void updateBackgroundAnimation();

private:
    void setupUI();
    void setupStyles();
    void setupAnimations();
    void createMenuButtons();
    void createBackground();
    void createHeader();
    void showLeaderboardDialog();    void showInstructionsDialog();
    void addButtonHoverEffect(QPushButton* button);

protected:
    void resizeEvent(QResizeEvent* event) override;
    
    // UI组件
    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QLabel* m_subtitleLabel;
    QLabel* m_welcomeLabel;
    QFrame* m_menuFrame;
    QVBoxLayout* m_menuLayout;
    
    // 菜单按钮
    QPushButton* m_startGameButton;
    QPushButton* m_leaderboardButton;
    QPushButton* m_instructionsButton;
    QPushButton* m_exitButton;
    
    // 背景动画
    QLabel* m_backgroundLabel;
    QTimer* m_backgroundTimer;
    int m_animationFrame;
    
    // 用户信息
    QString m_username;
    QSettings* m_settings;
    
    // 动画效果
    QPropertyAnimation* m_titleAnimation;
    QSequentialAnimationGroup* m_buttonAnimationGroup;
    
    // 游戏窗口
    MainWindow* m_gameWindow;
};

// 排行榜对话框
class LeaderboardDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit LeaderboardDialog(QWidget* parent = nullptr);
    
private:
    void setupUI();
    void loadLeaderboardData();
    
    QVBoxLayout* m_layout;
    QListWidget* m_leaderboardList;
    QPushButton* m_closeButton;
    QSettings* m_settings;
};

// 游戏说明对话框
class InstructionsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit InstructionsDialog(QWidget* parent = nullptr);
    
private:
    void setupUI();
    
    QVBoxLayout* m_layout;
    QTextBrowser* m_instructionsText;
    QPushButton* m_closeButton;
};
