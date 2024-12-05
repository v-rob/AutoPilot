#include "StrategyManager.h"

StrategyManager::StrategyManager() :
    m_productionManager(m_unitManager),
    m_buildingManager(m_unitManager),
    m_scoutManager(m_unitManager),
    m_combatManager(m_unitManager) {
}

void StrategyManager::notifyMembers(const bw::Event& event) {
    m_unitManager.notifyReceiver(event);

    m_productionManager.notifyReceiver(event);
    m_buildingManager.notifyReceiver(event);
    m_scoutManager.notifyReceiver(event);
    m_combatManager.notifyReceiver(event);
}

void StrategyManager::onStart() {
    // For now, we only have one predefined build order, but in the future we can make the
    // choice of build order take into account the race of our opponent.
    m_strategy = {
        {ActionType::TRAIN,  ActionItem::NONE, bw::UnitTypes::Protoss_Probe,        7 }, // 0
        {ActionType::BUILD,  0,                bw::UnitTypes::Protoss_Pylon,        1 }, // 1
        {ActionType::SCOUT,  1,                bw::UnitTypes::Protoss_Probe,        1 }, // 2
        {ActionType::BUILD,  1,                bw::UnitTypes::Protoss_Gateway,      1 }, // 3
        {ActionType::TRAIN,  3,                bw::UnitTypes::Protoss_Zealot,       2 }, // 4
        {ActionType::TRAIN,  3,                bw::UnitTypes::Protoss_Probe,        13}, // 5
        {ActionType::ATTACK, 4,                                                       }, // 6
    };

    // Make sure we reset the completion vector to all false values and resize it to be
    // the same size as the strategy vector.
    m_completion.clear();
    m_completion.resize(m_strategy.size());
}

void StrategyManager::onFrame() {
    // Loop through every item in the build order and see whether we need to execute it.
    for (int i = 0; i < m_strategy.size(); i++) {
        const ActionItem& item = m_strategy[i];

        // If this item's dependency (if any) hasn't been completed, then do nothing here.
        if (item.depends != ActionItem::NONE && !m_completion[item.depends]) {
            continue;
        }

        // Otherwise, we need to delegate an action to the appropriate manager class.
        switch (item.action) {
        case ActionType::BUILD:
        {
            // If the current number of buildings (including buildings that are only
            // partially constructed) meets the quota, then we're done.
            int current = m_unitManager.selfCount(bw::Filter::GetType == item.type);
            int progress = current + m_productionManager.countBuildRequests(item.type);

            m_completion[i] = current >= item.count;

            // Otherwise, try to build enough buildings up to the desired quota.
            while (progress < item.count && m_productionManager.addBuildRequest(item.type)) {
                progress++;
            }
            break;
        }

        case ActionType::TRAIN:
        {
            // If the number of fully trained units meets the quota, then we're done.
            int current = m_unitManager.selfCount(
                bw::Filter::GetType == item.type && bw::Filter::IsCompleted);
            int progress = m_unitManager.selfCount(bw::Filter::GetType == item.type);

            m_completion[i] = current >= item.count;

            // Otherwise, try to train enough units up to the desired quota.
            while (progress < item.count && m_buildingManager.addTrainRequest(item.type)) {
                progress++;
            }
            break;
        }

        case ActionType::SCOUT:
        {
            // If we currently have the required number of scouts, then we're done.
            int current = m_scoutManager.countScouts(item.type);

            m_completion[i] = current >= item.count;

            // Otherwise, add more scouts to fill up the quota of scouts in action.
            while (current < item.count && m_scoutManager.addScout(item.type)) {
                current++;
            }
            break;
        }

        case ActionType::ATTACK:
        {
            // If the attacking prerequisite has been satisfied, then we instruct the
            // combat manager to attack. For now, once the attack has started, it cannot
            // be stopped, so this action is permanently completed.
            if (!m_completion[i]) {
                m_combatManager.attack();
                m_completion[i] = true;
            }
            break;
        }
        }
    }
}