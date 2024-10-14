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
    // for now, just grab any any known enemy unit
    bw::Unit target = m_intelManager.findUnit([](const auto&) { return true; });
    chooseNewTarget(target);

    // reserve all nonworkers that can attack
    bw::Unitset army = m_unitManager.reserveUnits(bw::Filter::CanAttack && !bw::Filter::IsWorker);
    m_army.insert(army.begin(), army.end());

    m_attacking = true;
}

void CombatManager::onStart() {
    m_army.clear();
    m_targetBuildings.clear();
    m_target = nullptr;
    m_attacking = false;
}

void CombatManager::onFrame() {
    if (!m_attacking) {
        return;
    }

    // if target can't be found, select a new one
    if (m_intelManager.getLastPosition(m_target) == bw::TilePositions::Unknown) {
        m_targetBuildings.erase(m_target);

        if (m_targetBuildings.size() != 0) {
            m_target = *m_targetBuildings.begin();
        }
    }

    bw::TilePosition targetPosition = m_intelManager.getLastPosition(m_target);
    m_army.attack(bw::Position(targetPosition));
}
