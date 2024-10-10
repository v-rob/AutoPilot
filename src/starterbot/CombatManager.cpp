#include "CombatManager.h"


CombatManager::CombatManager(UnitManager& unitManager, IntelManager& intelManager) :
    m_unitManager(unitManager), m_intelManager(intelManager), m_targetRadius(50) {
}

void CombatManager::chooseNewTarget(bw::Unit target) {
    m_target = target;

    bw::Unitset enemyBuildings = m_intelManager.findUnits(bw::Filter::IsBuilding);

    // set all enemy buildings close by as targets
    for (bw::Unit building : enemyBuildings) {
        bw::TilePosition position = m_intelManager.getLastPosition(building);

        if (position == bw::TilePositions::Unknown) {
            continue;
        }

        if (target->getDistance(bw::Position(position)) < m_targetRadius) {
            m_targetBuildings.insert(building);
        }
    }
}

void CombatManager::attack() {
    // for now, just grab any known enemy building
    bw::Unit target = m_intelManager.findUnit(true);
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

    // if target is dead or can't be found, select a new one
    if (!m_target->isVisible() || m_intelManager.getLastPosition(m_target) == bw::TilePositions::Unknown) {
        m_targetBuildings.erase(m_target);

        if (m_targetBuildings.size() != 0) {
            m_target = *m_targetBuildings.begin();
        }
    }

    m_army.attack(m_target);
}
