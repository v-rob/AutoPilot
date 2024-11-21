#include "CombatManager.h"

#include "UnitTools.h"

CombatManager::CombatManager(UnitManager& unitManager) :
    m_unitManager(unitManager) {
}

void CombatManager::attack() {
    m_attacking = true;
}

void CombatManager::onStart() {
    m_attacking = false;
    m_soldiers.clear();
}

void CombatManager::onFrame() {
    if (!m_attacking) {
        return;
    }

    // Reserve every single unit that can attack that is not already reserved and add them
    // to our army.
    bw::Unitset newSoldiers = m_unitManager.reserveUnits(bw::Filter::CanAttack);
    m_soldiers.insert(newSoldiers.begin(), newSoldiers.end());
}

void CombatManager::onUnitDestroy(bw::Unit unit) {
    m_soldiers.erase(unit);
}