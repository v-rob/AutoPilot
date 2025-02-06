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
    // For now, we only have one predefined build order per race, but in the future we can
    // make the choice of build order take into account the race of our opponent.
    if (g_self->getRace() == bw::Races::Protoss) {
        m_strategy = {
            {ActionType::TRAIN,   /* 0 */   -1,  bw::UnitTypes::Protoss_Probe,        7 },
            {ActionType::BUILD,   /* 1 */   0,   bw::UnitTypes::Protoss_Pylon,        1 },
            {ActionType::SCOUT,   /* 2 */   1,   bw::UnitTypes::Protoss_Probe,        1 },
            {ActionType::TRAIN,   /* 3 */   2,   bw::UnitTypes::Protoss_Probe,        13},
            {ActionType::BUILD,   /* 4 */   2,   bw::UnitTypes::Protoss_Gateway,      1 },
            {ActionType::TRAIN,   /* 5 */   4,   bw::UnitTypes::Protoss_Zealot,       2 },
            {ActionType::ATTACK,  /* 6 */   5,                                          },
        };
    } else if (g_self->getRace() == bw::Races::Terran) {
        m_strategy = {
            {ActionType::TRAIN,   /* 0 */   -1,  bw::UnitTypes::Terran_SCV,           8 },
            {ActionType::BUILD,   /* 1 */   0,   bw::UnitTypes::Terran_Supply_Depot,  1 },
            {ActionType::SCOUT,   /* 2 */   1,   bw::UnitTypes::Terran_SCV,           1 },
            {ActionType::TRAIN,   /* 3 */   2,   bw::UnitTypes::Terran_SCV,           16},
            {ActionType::BUILD,   /* 4 */   2,   bw::UnitTypes::Terran_Barracks,      1 },
            {ActionType::TRAIN,   /* 5 */   4,   bw::UnitTypes::Terran_Marine,        2 },
            {ActionType::ATTACK,  /* 6 */   5,                                          },
        };
    } else {
        m_strategy = {
            {ActionType::MORPH,   /* 0 */   -1,  bw::UnitTypes::Zerg_Drone,           9 },
            {ActionType::BUILD,   /* 1 */   0,   bw::UnitTypes::Zerg_Spawning_Pool,   1 },
            {ActionType::SCOUT,   /* 2 */   1,   bw::UnitTypes::Zerg_Drone,           1 },
            {ActionType::MORPH,   /* 3 */   1,   bw::UnitTypes::Zerg_Overlord,        2 },
            {ActionType::MORPH,   /* 4 */   3,   bw::UnitTypes::Zerg_Drone,           13},
            {ActionType::MORPH,   /* 5 */   1,   bw::UnitTypes::Zerg_Zergling,        2 },
            {ActionType::ATTACK,  /* 6 */   5,                                          },
        };
    }

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
        if (item.depends != -1 && !m_completion[item.depends]) {
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

        case ActionType::MORPH:
        {
            // If the number of fully morphed units meets the quota, then we're done.
            int current = m_unitManager.selfCount(bw::Filter::GetType == item.type);
            int progress = m_unitManager.selfCount(
                bw::Filter::GetType == item.type || bw::Filter::BuildType == item.type);

            m_completion[i] = current >= item.count;

            // Otherwise, try to morph enough units up to the desired quota.
            while (progress < item.count && m_productionManager.addMorphRequest(item.type)) {
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