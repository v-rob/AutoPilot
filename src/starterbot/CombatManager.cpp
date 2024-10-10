#include "CombatManager.h"


CombatManager::CombatManager(IntelManager& intelManager) :
    m_intelManager(intelManager), m_targetRadius(50) {
}

void CombatManager::chooseNewTarget(bw::Unit target) {
    m_target = target;

    for (const auto& [unit, position] : m_intelManager.m_enemyBuildings) {
        if (position == bw::Positions::Unknown) {
            continue;
        }

        if (target->getDistance(position) < m_targetRadius) {
            m_targetBuildings.insert(unit);
        }
    }
}

void CombatManager::attack() {
    // for now, just grab any known enemy building
    bw::Unit target = m_intelManager.m_enemyBuildings.begin();
    chooseNewTarget(target);

    // reserve all units that can attack
    bw::Unitset army = m_unitManager.reserveUnits(bw::Filter::CanAttack);
    m_army.insert(army.begin(), army.end());

    m_attacking = true;
}


void CombatManager::onFrame() {
    if (!m_attacking) {
        return;
    }

    if (m_intelManager.m_enemyBuildings.find(m_target) == bw::Positions::Unknown) {
        m_targetBuildings.erase(m_target);
        m_target = m_targetBuildings.begin();
    }

    m_army.attack(m_target);
}
