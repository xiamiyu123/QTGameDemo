#include "player.h"
#include "debuglogger.h"
#include <QGraphicsScene>

// NPCCarryManager方法实现

void NPCCarryManager::addNPC(NPCEntity* npc) {
    if (!npc) return;
    
    // 已经携带则不再添加
    if (m_npcSet.find(npc) != m_npcSet.end()) {
        return;
    }
    
    // 添加到优先队列和集合
    CarriedNPC carriedNPC(npc, npc->getCarryEffect(), npc->getPriority());
    m_carriedNPCs.push(carriedNPC);
    m_npcSet.insert(npc);
    
    // 标记NPC为已携带状态
    npc->setCarried(true);
    
    DEBUG_LOG(QString("玩家拾取NPC，优先级: %1, 总计已携带: %2")
              .arg(npc->getPriority())
              .arg(m_carriedNPCs.size()));
}

bool NPCCarryManager::removeNPC(NPCEntity* npc) {
    // 如果NPC不在集合中，直接返回false
    if (m_npcSet.find(npc) == m_npcSet.end()) {
        return false;
    }
    
    // 从集合中移除
    m_npcSet.erase(npc);
    
    // 标记NPC为非携带状态
    npc->setCarried(false);
    
    // 重建优先队列（因为优先队列不支持直接删除）
    std::priority_queue<CarriedNPC> newQueue;
    std::vector<NPCEntity*> tempNPCs;
    
    // 保存所有NPC
    while (!m_carriedNPCs.empty()) {
        NPCEntity* currentNPC = m_carriedNPCs.top().npc;
        if (currentNPC != npc) {
            tempNPCs.push_back(currentNPC);
        }
        m_carriedNPCs.pop();
    }
    
    // 重新添加所有NPC到新队列
    for (NPCEntity* currentNPC : tempNPCs) {
        CarriedNPC carriedNPC(currentNPC, currentNPC->getCarryEffect(), currentNPC->getPriority());
        newQueue.push(carriedNPC);
    }
    
    // 替换原队列
    m_carriedNPCs = std::move(newQueue);
    
    DEBUG_LOG(QString("玩家丢弃NPC，剩余携带: %1").arg(m_carriedNPCs.size()));
    return true;
}

NPCEntity* NPCCarryManager::getHighestPriorityNPC() const {
    if (m_carriedNPCs.empty()) {
        return nullptr;
    }
    return m_carriedNPCs.top().npc;
}

NPCEntity* NPCCarryManager::getLowestPriorityNPC() const {
    if (m_carriedNPCs.empty()) {
        return nullptr;
    }
    
    // 找到优先级最低的NPC
    NPCEntity* lowestPriorityNPC = nullptr;
    int lowestPriority = INT_MAX;
    
    // 临时保存所有NPC
    std::vector<NPCEntity*> tempNPCs;
    std::priority_queue<CarriedNPC> tempQueue = m_carriedNPCs; // 复制队列
    
    while (!tempQueue.empty()) {
        NPCEntity* currentNPC = tempQueue.top().npc;
        int currentPriority = tempQueue.top().priority;
        
        if (currentPriority < lowestPriority) {
            lowestPriority = currentPriority;
            lowestPriorityNPC = currentNPC;
        }
        
        tempQueue.pop();
    }
    
    return lowestPriorityNPC;
}

NPCCarryEffect NPCCarryManager::getHighestPriorityEffect() const {
    if (m_carriedNPCs.empty()) {
        return NPCCarryEffect(); // 返回默认效果
    }
    return m_carriedNPCs.top().effect;
}

NPCCarryEffect NPCCarryManager::getTotalEffect() const {
    if (m_carriedNPCs.empty()) {
        return NPCCarryEffect(); // 返回默认效果
    }
    
    // 创建基础效果
    NPCCarryEffect totalEffect;
    
    // 临时保存所有NPC
    std::priority_queue<CarriedNPC> tempQueue = m_carriedNPCs; // 复制队列
    
    while (!tempQueue.empty()) {
        const NPCCarryEffect& currentEffect = tempQueue.top().effect;
        
        // 累加叠加效果
        totalEffect.speedMultiplier *= currentEffect.speedMultiplier;
        totalEffect.jumpForceMultiplier *= currentEffect.jumpForceMultiplier;
        totalEffect.gravityMultiplier *= currentEffect.gravityMultiplier;
        
        // 取最高的旋转阻力
        totalEffect.rotationResistance = 
            qMax(totalEffect.rotationResistance, currentEffect.rotationResistance);
        
        // 按位OR特殊效果标识
        totalEffect.enableDoubleJump = totalEffect.enableDoubleJump || currentEffect.enableDoubleJump;
        totalEffect.enableGliding = totalEffect.enableGliding || currentEffect.enableGliding;
        totalEffect.immuneToFall = totalEffect.immuneToFall || currentEffect.immuneToFall;
        
        // 记录效果描述
        if (!currentEffect.effectDescription.isEmpty()) {
            if (!totalEffect.effectDescription.isEmpty()) {
                totalEffect.effectDescription += ", ";
            }
            totalEffect.effectDescription += currentEffect.effectDescription;
        }
        
        tempQueue.pop();
    }
    
    return totalEffect;
}

void NPCCarryManager::clear() {
    // 将所有NPC标记为非携带状态
    while (!m_carriedNPCs.empty()) {
        NPCEntity* npc = m_carriedNPCs.top().npc;
        if (npc) {
            npc->setCarried(false);
        }
        m_carriedNPCs.pop();
    }
    
    // 清空集合
    m_npcSet.clear();
    DEBUG_LOG("玩家清空所有携带的NPC");
}

bool NPCCarryManager::isCarrying(NPCEntity* npc) const {
    return m_npcSet.find(npc) != m_npcSet.end();
}

std::vector<NPCEntity*> NPCCarryManager::getAllCarriedNPCs() const {
    std::vector<NPCEntity*> result;
    result.reserve(m_npcSet.size());
    
    // 直接从集合中获取所有NPC
    for (NPCEntity* npc : m_npcSet) {
        result.push_back(npc);
    }
    
    return result;
}
