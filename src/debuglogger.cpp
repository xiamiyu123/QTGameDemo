#include "debuglogger.h"
#include <QGraphicsScene>
#include <QDateTime>
#include <QMutexLocker>
#include <QGraphicsView>
#include <QFontMetrics>
#include <QScrollBar>
#include <QDir>

// 初始化静态实例
DebugLogger* DebugLogger::s_instance = nullptr;

DebugLogger* DebugLogger::instance()
{
    if (!s_instance) {
        s_instance = new DebugLogger();
    }
    return s_instance;
}

void DebugLogger::destroyInstance()
{
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

DebugLogger::DebugLogger(QObject* parent)
    : QObject(parent)
    , m_scene(nullptr)
    , m_logTextItem(nullptr)
    , m_visible(false)
    , m_initialized(false)
    , m_overlayTextEdit(nullptr)
{
    // 配置更新定时器
    connect(&m_updateTimer, &QTimer::timeout, this, &DebugLogger::updateDisplay);
    m_updateTimer.setInterval(500); // 每半秒更新一次
    
    // 初始化日志文件
    initLogFile();
}

DebugLogger::~DebugLogger()
{
    // 停止定时器
    m_updateTimer.stop();
    
    // 移除文本项
    if (m_scene && m_logTextItem) {
        m_scene->removeItem(m_logTextItem);
        delete m_logTextItem;
        m_logTextItem = nullptr;
    }
    
    // 删除文本编辑框
    if (m_overlayTextEdit) {
        m_overlayTextEdit->deleteLater();
        m_overlayTextEdit = nullptr;
    }
    
    // 关闭日志文件
    if (m_logFile.isOpen()) {
        m_logStream.flush();
        m_logFile.close();
    }
}

void DebugLogger::initialize(QGraphicsScene* scene)
{
    m_scene = scene;
    
    // 使用新方法创建视口覆盖层
    createOrUpdateViewportText();
    
    // 标记为已初始化
    m_initialized = true;
    
    // 启动定时器
    m_updateTimer.start();
}

void DebugLogger::log(const QString& message)
{
    // 获取当前时间
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString formattedMessage = QString("[%1] %2").arg(timestamp).arg(message);
    
    // 线程安全地添加日志
    {
        QMutexLocker locker(&m_mutex);
        
        // 添加日志到队列
        m_logs.enqueue(formattedMessage);
        
        // 保持日志数量在最大限制以内
        while (m_logs.size() > MAX_LOG_LINES) {
            m_logs.dequeue();
        }
        
        // 写入日志文件
        if (m_logFile.isOpen()) {
            m_logStream << formattedMessage << Qt::endl;
            m_logStream.flush();
        }
    }
    
    // 发送日志添加信号
    emit newLogAdded(formattedMessage);
}

void DebugLogger::setVisible(bool visible)
{
    m_visible = visible;
    if (m_overlayTextEdit) {
        m_overlayTextEdit->setVisible(visible);
    }
}

void DebugLogger::updatePosition()
{
    // 使用新方法更新位置
    createOrUpdateViewportText();
}

void DebugLogger::updateDisplay()
{
    if (!m_overlayTextEdit) return;
    
    QMutexLocker locker(&m_mutex);
    
    // 保存当前滚动位置
    QScrollBar* vScrollBar = m_overlayTextEdit->verticalScrollBar();
    bool wasAtBottom = vScrollBar->value() == vScrollBar->maximum();
    
    // 构建纯文本格式日志
    QString text;
    foreach (const QString& log, m_logs) {
        text += log + "\n";
    }
    
    // 设置文本
    m_overlayTextEdit->setText(text);
    
    // 如果之前在底部，则保持在底部（自动滚动）
    if (wasAtBottom) {
        vScrollBar->setValue(vScrollBar->maximum());
    }
    
    // 更新位置，确保始终固定在视口
    updatePosition();
}

// 创建或更新视口固定文本项
void DebugLogger::createOrUpdateViewportText()
{
    if (!m_scene || m_scene->views().isEmpty()) return;

    // 获取视图
    QGraphicsView* view = m_scene->views().first();
    if (!view) return;
    
    // 如果尚未创建文本编辑框，则创建
    if (!m_overlayTextEdit) {
        m_overlayTextEdit = new QTextEdit(view->viewport());
        m_overlayTextEdit->setStyleSheet("QTextEdit { color: white; background-color: rgba(0, 0, 0, 120); padding: 5px; border-radius: 5px; border: none; }");
        m_overlayTextEdit->setFont(QFont("Consolas", 10));
        m_overlayTextEdit->setReadOnly(true); // 只允许选择和复制，不允许编辑
        m_overlayTextEdit->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        m_overlayTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_overlayTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        
        // 设置大小
        m_overlayTextEdit->setMinimumSize(400, 150);
        m_overlayTextEdit->setMaximumSize(400, 200);
        
        // 设置可见性
        m_overlayTextEdit->setVisible(m_visible);
    }
    
    // 确保文本编辑框已添加到视口
    m_overlayTextEdit->setParent(view->viewport());
    
    // 设置位置 - 左下角
    QSize viewportSize = view->viewport()->size();
    QSize textEditSize = m_overlayTextEdit->size();
    
    int marginX = 20;
    int marginY = 20;
    
    // 左下角位置
    m_overlayTextEdit->move(marginX, viewportSize.height() - textEditSize.height() - marginY);
    
    // 确保文本编辑框在最前面
    m_overlayTextEdit->raise();
}

// 切换调试信息显示状态
void DebugLogger::toggleVisibility()
{
    setVisible(!m_visible);
}

// 获取当前显示状态
bool DebugLogger::isVisible() const
{
    return m_visible;
}

// 初始化日志文件
void DebugLogger::initLogFile()
{
    // 创建日志目录（如果不存在）
    QDir dir;
    if (!dir.exists("logs")) {
        dir.mkdir("logs");
    }
    
    // 使用当前日期和时间作为文件名，确保每次运行都创建新文件
    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    QString logFileName = QString("logs/debug_log_%1.txt").arg(currentDateTime);
    
    // 打开日志文件
    m_logFile.setFileName(logFileName);
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_logStream.setDevice(&m_logFile);
        
        // 写入日志头部信息
        m_logStream << "===== Debug Log Started: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << " =====" << Qt::endl;
        m_logStream.flush();
    } else {
        qWarning() << "Failed to open log file:" << logFileName;
    }
}
