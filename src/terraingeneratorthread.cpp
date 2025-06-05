#include "terraingeneratorthread.h"
#include "terraingenerator.h"
#include "debuglogger.h"

TerrainGeneratorThread::TerrainGeneratorThread(TerrainGenerator* terrainGenerator, QObject* parent)
    : QThread(parent), m_terrainGenerator(terrainGenerator), m_abort(false)
{
    DEBUG_LOG("地形生成进程启动");
}

TerrainGeneratorThread::~TerrainGeneratorThread()
{
    // 确保线程安全退出
    stop();
    wait();
}

void TerrainGeneratorThread::stop()
{
    // 加锁修改共享变量
    QMutexLocker locker(&m_mutex);
    m_abort = true;
    // 唤醒所有等待的线程
    m_condition.wakeAll();
}

void TerrainGeneratorThread::requestChunkGeneration(int chunkIndex)
{
    // 加锁修改共享变量
    QMutexLocker locker(&m_mutex);
    
    // 如果这个区块不在队列中，添加到队列
    if (!m_pendingChunks.contains(chunkIndex)) {
        m_pendingChunks.append(chunkIndex);
        // 唤醒等待的线程
        m_condition.wakeOne();
    }
}

void TerrainGeneratorThread::run()
{
    // 线程主循环
    forever {
        // 获取下一个要生成的区块
        int nextChunk = 0;
        {
            // 加锁访问共享数据
            QMutexLocker locker(&m_mutex);
            
            // 如果没有待处理的区块且未请求停止，则等待
            if (m_pendingChunks.isEmpty()) {
                // 等待新的生成请求或停止信号
                m_condition.wait(&m_mutex);
            }
            
            // 检查是否请求终止线程
            if (m_abort) {
                return;
            }
            
            // 如果队列不为空，取出第一个区块
            if (!m_pendingChunks.isEmpty()) {
                nextChunk = m_pendingChunks.takeFirst();
            } else {
                continue; // 如果队列为空（被唤醒但没有任务），继续循环
            }
        } // 锁在这里释放

        // 生成区块 - 不需要锁，因为我们不访问共享数据
        m_terrainGenerator->generateChunkThreadSafe(nextChunk);

        // 发送信号通知区块生成完成
        emit chunkGenerated(nextChunk);
    }
}