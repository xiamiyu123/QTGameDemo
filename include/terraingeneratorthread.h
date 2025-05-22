#pragma once

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

// 使用前向声明
class TerrainGenerator;

class TerrainGeneratorThread : public QThread
{
    Q_OBJECT

public:
    // 构造函数，传入TerrainGenerator指针
    TerrainGeneratorThread(TerrainGenerator* terrainGenerator, QObject* parent = nullptr);
    ~TerrainGeneratorThread();

    // 请求生成某个区块
    void requestChunkGeneration(int chunkIndex);
    
    // 停止线程
    void stop();

protected:
    // 线程执行的主函数
    void run() override;

signals:
    // 当区块生成完成时发送信号
    void chunkGenerated(int chunkIndex);

private:
    TerrainGenerator* m_terrainGenerator; // 地形生成器
    QList<int> m_pendingChunks;          // 等待生成的区块队列
    QMutex m_mutex;                      // 互斥锁，保护共享数据
    QWaitCondition m_condition;          // 等待条件，用于线程同步
    bool m_abort;                        // 控制线程终止
};
