#include "avalancheupdatethread.h"
#include "debuglogger.h"

AvalancheUpdateThread::AvalancheUpdateThread(Avalanche* avalanche, QObject* parent)
    : QThread(parent), m_avalanche(avalanche), m_abort(false), m_updatePending(false)
{
    DEBUG_LOG("雪崩更新进程启动");
}

AvalancheUpdateThread::~AvalancheUpdateThread()
{
    stop();
    wait();
}

void AvalancheUpdateThread::stop()
{
    QMutexLocker locker(&m_mutex);
    m_abort = true;
    m_condition.wakeAll();
}

void AvalancheUpdateThread::requestUpdate(qreal elapsed, qreal playerX)
{
    QMutexLocker locker(&m_mutex);
    
    // 保存更新参数
    m_elapsed = elapsed;
    m_playerX = playerX;
    m_updatePending = true;
    
    // 唤醒等待的线程
    m_condition.wakeOne();
}

void AvalancheUpdateThread::run()
{
    forever {
        // 线程开始等待
        {
            QMutexLocker locker(&m_mutex);
            
            // 如果没有更新请求且未停止，则等待
            if (!m_updatePending && !m_abort) {
                m_condition.wait(&m_mutex);
            }
            
            // 检查是否要退出线程
            if (m_abort) {
                return;
            }
            
            // 如果没有待处理的更新，继续等待
            if (!m_updatePending) {
                continue;
            }
            
            // 标记为已处理
            m_updatePending = false;
        } // 锁在这里释放
        
        // 执行雪崩更新（雪崩对象需要线程安全）
        m_avalanche->updateAvalancheThreadSafe(m_elapsed, m_playerX);
        
        // 发送更新完成的信号
        emit updateCompleted();
    }
}