#include "StrategyManager.h"

StrategyManager::StrategyManager() :
    m_productionManager(m_unitManager),
    m_buildingManager(m_unitManager),
    m_scoutManager(m_unitManager) {
}

void StrategyManager::notifyMembers(const bw::Event& event) {
    m_unitManager.notifyReceiver(event);
    m_intelManager.notifyReceiver(event);

    m_productionManager.notifyReceiver(event);
    m_buildingManager.notifyReceiver(event);
    m_scoutManager.notifyReceiver(event);
}

void StrategyManager::onStart() {
    m_strategy = {
        {ActionType::TRAIN,  ActionItem::NONE, bw::UnitTypes::Protoss_Probe,        8 }, // 0
        {ActionType::BUILD,  0,                bw::UnitTypes::Protoss_Pylon,        1 }, // 1
        {ActionType::TRAIN,  1,                bw::UnitTypes::Protoss_Probe,        9 }, // 2
        {ActionType::BUILD,  2,                bw::UnitTypes::Protoss_Gateway,      1 }, // 3
        {ActionType::BUILD,  3,                bw::UnitTypes::Protoss_Gateway,      2 }, // 4
        {ActionType::SCOUT,  4,                bw::UnitTypes::Protoss_Probe,        1 }, // 5
        {ActionType::TRAIN,  3,                bw::UnitTypes::Protoss_Zealot,       4 }, // 6
        {ActionType::BUILD,  5,                bw::UnitTypes::Protoss_Pylon,        2 }, // 7
        {ActionType::TRAIN,  5,                bw::UnitTypes::Protoss_Probe,        16}, // 0
        {ActionType::ATTACK, 6,                                                       }, // 9
    };

    m_completion.clear();
    m_completion.resize(m_strategy.size());
}

void StrategyManager::onFrame() {
    for (int i = 0; i < m_strategy.size(); i++) {
        const ActionItem& item = m_strategy[i];

        if (item.depends != ActionItem::NONE && !m_completion[item.depends]) {
            continue;
        }

        switch (item.action) {
        case ActionType::BUILD:
        {
            int current = m_unitManager.peekCount(bw::Filter::GetType == item.type, true);
            int progress = current + m_productionManager.countBuildRequests(item.type);

            m_completion[i] = current >= item.count;

            while (progress < item.count && m_productionManager.addBuildRequest(item.type)) {
                progress++;
            }
            break;
        }

        case ActionType::TRAIN:
        {
            int current = m_unitManager.peekCount(bw::Filter::GetType == item.type, false);
            int progress = m_unitManager.peekCount(bw::Filter::GetType == item.type, true);

            m_completion[i] = current >= item.count;

            while (progress < item.count && m_buildingManager.addTrainRequest(item.type)) {
                progress++;
            }
            break;
        }

        case ActionType::SCOUT:
        {
            int current = m_scoutManager.countScouts(item.type);

            m_completion[i] = current >= item.count;

            while (current < item.count && m_scoutManager.addScout(item.type)) {
                current++;
            }
            break;
        }

        case ActionType::ATTACK:
            // TODO
            break;
        }
    }
}