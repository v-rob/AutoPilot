#include "StrategyManager.h"

void StrategyManager::notifyMembers(const bw::Event &event) {
    m_unitManager.notifyReceiver(event);
}

void StrategyManager::onStart() {
    m_strategy = {
        {ActionType::TRAIN, bw::UnitTypes::Protoss_Probe, 10},
        {ActionType::BUILD, bw::UnitTypes::Protoss_Pylon, 1},
    };
    m_strategyItem = 0;
}

void StrategyManager::onFrame() {
    // TODO: We need ProductionManager and BuildingManager before this can be implemented.
}