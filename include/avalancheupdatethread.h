#pragma once
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "avalanche.h"

class AvalancheUpdateThread : public QThread
{
    Q_OBJECT

public:
    AvalancheUpdateThread(Avalanche* avalanche, QObject* parent = nullptr);
    ~AvalancheUpdateThread();
    
    // 请求更新雪崩状态
    void requestUpdate(qreal elapsed, qreal playerX);
    
    // 停止线程
    void stop();

protected:
    // 线程主函数
    void run() override;

signals:
    // 当更新完成时发出信号
    void updateCompleted();

private:
    Avalanche* m_avalanche;        // 雪崩对象的指针
    QMutex m_mutex;               // 互斥锁保护共享数据
    QWaitCondition m_condition;   // 等待条件，用于线程同步
    bool m_abort;                 // 控制线程终止
    
    // 更新参数
    qreal m_elapsed;              // 经过的时间
    qreal m_playerX;              // 玩家位置
    bool m_updatePending;         // 是否有待处理的更新
};