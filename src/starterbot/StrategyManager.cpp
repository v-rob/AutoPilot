#include "StrategyManager.h"

StrategyManager::StrategyManager() :
    m_productionManager(m_unitManager),
    m_buildingManager(m_unitManager) {
}

void StrategyManager::notifyMembers(const bw::Event& event) {
    m_unitManager.notifyReceiver(event);
    m_productionManager.notifyReceiver(event);
    m_buildingManager.notifyReceiver(event);
}

void StrategyManager::onStart() {
    m_strategy = {
        {ActionType::TRAIN, bw::UnitTypes::Protoss_Probe, 8},
        {ActionType::BUILD, bw::UnitTypes::Protoss_Pylon, 1},
        {ActionType::TRAIN, bw::UnitTypes::Protoss_Probe, 15},
        {ActionType::BUILD, bw::UnitTypes::Protoss_Gateway, 1},
        {ActionType::TRAIN, bw::UnitTypes::Protoss_Zealot, 5},
    };
    m_strategyItem = 0;
}

void StrategyManager::onFrame() {
    if (m_strategyItem >= m_strategy.size()) {
        return;
    }

    const ActionItem& item = m_strategy[m_strategyItem];

    switch (item.type) {
    case ActionType::BUILD: {
            int current = m_unitManager.peekCount(bw::Filter::GetType == item.unit, false);
            int progress = m_unitManager.peekCount(bw::Filter::GetType == item.unit, true) +
                m_productionManager.countBuildRequests(item.unit);

            if (current >= item.count) {
                m_strategyItem++;
            }

            while (progress < item.count && m_productionManager.addBuildRequest(item.unit)) {
                printf("Building\n");
                progress++;
            }
            
            break;
        }

    case ActionType::TRAIN: {
        int current = m_unitManager.peekCount(bw::Filter::GetType == item.unit, false);
        int progress = m_unitManager.peekCount(bw::Filter::GetType == item.unit, true);

        if (current >= item.count) {
            m_strategyItem++;
        }

        while (progress < item.count && m_buildingManager.addTrainRequest(item.unit)) {
            progress++;
        }
        break;
    }

    case ActionType::SCOUT:
        // TODO
        break;

    case ActionType::ATTACK:
        // TODO
        break;
    }
}