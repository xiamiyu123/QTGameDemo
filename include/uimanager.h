#pragma once

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QPushButton>

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

    // 设置与获取
    QPushButton* getPauseButton() const { return m_pauseButton; }
    QPushButton* getWarningButton() const { return m_warningButton; }
    bool isPauseTextVisible() const { return m_pauseText && m_pauseText->isVisible(); }

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

    // 创建UI元素的辅助方法
    void createPauseElements();
    void createWarningElements();
    void setupButtonStyle(QPushButton* button, const QString& iconPath, bool transparent = true);

    // 获取主视图
    QGraphicsView* getView() const;
};
