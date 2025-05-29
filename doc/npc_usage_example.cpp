// npc_usage_example.cpp
// 这是一个使用示例文件，展示如何使用改进后的NPC系统
// 您可以参考这些代码片段在GameScene中集成NPC

#include "npcentity.h"
#include "groundnpc.h"
#include "flyingnpc.h"
#include <QGraphicsScene>
#include <QTimer>
#include <vector>
#include <memory>

// === 在GameScene中使用NPC的示例 ===

class NPCManager : public QObject 
{
    Q_OBJECT

public:
    explicit NPCManager(QGraphicsScene* scene, QObject* parent = nullptr) 
        : QObject(parent), m_scene(scene) {}

    // 生成NPC的示例方法
    void spawnRandomNPC(const QPointF& position) {
        // 随机选择NPC类型
        bool isFlying = (QRandomGenerator::global()->bounded(2) == 0);
        
        std::unique_ptr<NPCEntity> npc;
        if (isFlying) {
            npc = NPCFactory::createFlyingNPC(position);
        } else {
            npc = NPCFactory::createGroundNPC(position);
        }
        
        // 添加到场景
        m_scene->addItem(npc.get());
        
        // 管理NPC生命周期
        m_activeNPCs.push_back(std::move(npc));
    }

    // 更新所有NPC
    void updateAllNPCs(float deltaTime) {
        // 遍历并更新所有活跃的NPC
        for (auto it = m_activeNPCs.begin(); it != m_activeNPCs.end();) {
            auto& npc = *it;
            
            if (npc->shouldDestroy()) {
                // 从场景中移除
                m_scene->removeItem(npc.get());
                // 从容器中删除
                it = m_activeNPCs.erase(it);
            } else {
                // 更新NPC
                npc->updatePhysics(deltaTime);
                ++it;
            }
        }
    }

    // 清理所有NPC
    void clearAllNPCs() {
        for (auto& npc : m_activeNPCs) {
            m_scene->removeItem(npc.get());
        }
        m_activeNPCs.clear();
    }

    // 获取NPC数量
    size_t getNPCCount() const {
        return m_activeNPCs.size();
    }

private:
    QGraphicsScene* m_scene;
    std::vector<std::unique_ptr<NPCEntity>> m_activeNPCs;
};

// === 在GameScene中集成的示例代码片段 ===

/*
// 在GameScene的私有成员中添加：
NPCManager* m_npcManager;
QTimer* m_npcSpawnTimer;

// 在GameScene::initialize()中初始化：
void GameScene::initialize() {
    // ...现有代码...
    
    // 初始化NPC管理器
    m_npcManager = new NPCManager(this, this);
    
    // 设置NPC生成定时器
    m_npcSpawnTimer = new QTimer(this);
    connect(m_npcSpawnTimer, &QTimer::timeout, this, &GameScene::spawnNPC);
    m_npcSpawnTimer->start(3000); // 每3秒生成一个NPC
}

// 在GameScene::update()中更新NPC：
void GameScene::update() {
    // ...现有代码...
    
    // 更新NPC
    float deltaTime = GElapsedTimer.elapsed() / 1000.0f;
    m_npcManager->updateAllNPCs(deltaTime);
}

// 生成NPC的槽函数：
void GameScene::spawnNPC() {
    // 在玩家前方生成NPC
    QPointF playerPos = Gplayer->position();
    QPointF spawnPos(playerPos.x() + 800, playerPos.y() - 100); // 在玩家前方800像素处
    
    m_npcManager->spawnRandomNPC(spawnPos);
}

// 在游戏重置时清理NPC：
void GameScene::resetGameState() {
    // ...现有代码...
    
    if (m_npcManager) {
        m_npcManager->clearAllNPCs();
    }
}
*/
