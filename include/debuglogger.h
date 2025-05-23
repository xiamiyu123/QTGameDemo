#pragma once

#include <QObject>
#include <QGraphicsTextItem>
#include <QString>
#include <QQueue>
#include <QTimer>
#include <QMutex>
#include <QGraphicsView>
#include <QTextEdit>

// 调试日志记录器类
class DebugLogger : public QObject
{
    Q_OBJECT

public:
    // 单例获取方法
    static DebugLogger* instance();
    
    // 销毁单例
    static void destroyInstance();
    
    // 初始化调试器
    void initialize(QGraphicsScene* scene);
    
    // 添加日志
    void log(const QString& message);
    
    // 设置是否显示日志
    void setVisible(bool visible);
    
    // 更新UI位置
    void updatePosition();
    
    // 切换显示状态
    void toggleVisibility();
    
    // 获取当前显示状态
    bool isVisible() const;
    
signals:
    // 新日志信号
    void newLogAdded(const QString& message);

private slots:
    // 刷新显示
    void updateDisplay();

private:
    // 私有构造函数，实现单例模式
    explicit DebugLogger(QObject* parent = nullptr);
    ~DebugLogger();
    
    // 单例实例
    static DebugLogger* s_instance;
    
    // 场景指针
    QGraphicsScene* m_scene;
    
    // 日志文本项
    QGraphicsTextItem* m_logTextItem;
    
    // 日志队列
    QQueue<QString> m_logs;
    
    // 更新定时器
    QTimer m_updateTimer;
    
    // 同步锁
    QMutex m_mutex;    // 最大日志行数
    static const int MAX_LOG_LINES = 30;
    
    // 是否显示日志
    bool m_visible;
    
    // 是否创建了文本项
    bool m_initialized;
      // 视口叠加层文本编辑框
    QTextEdit* m_overlayTextEdit;
    
    // 创建或更新视口固定文本项
    void createOrUpdateViewportText();
};

// 定义便捷的日志宏，用于替代qDebug
#define DEBUG_LOG(msg) DebugLogger::instance()->log(msg)
