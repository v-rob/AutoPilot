#include "CombatManager.h"

CombatManager::CombatManager(UnitManager& unitManager) :
    m_unitManager(unitManager) {
}

void CombatManager::attack() {
    m_attacking = true;
}

void CombatManager::onStart() {
    m_attacking = false;
    m_soldiers.clear();

    m_target = nullptr;
}

void CombatManager::onFrame() {
    if (!m_attacking) {
        return;
    }

    // Reserve every single unit that can attack that is not already reserved and add them
    // to our army.
    bw::Unitset newSoldiers = m_unitManager.reserveUnits(bw::Filter::CanAttack);
    m_soldiers.insert(newSoldiers.begin(), newSoldiers.end());

    // If we have a target that is no longer alive, we need to choose a new one.
    if (m_target != nullptr && !m_unitManager.isAlive(m_target)) {
        m_target = nullptr;
    }

    // If we don't have a current target or our target is not currently visible, try to
    // find a visible unit to attack.
    if (m_target == nullptr || !m_target->isVisible()) {
        bw::Unit possible = m_unitManager.enemyUnit(bw::Filter::IsVisible);
        if (possible != nullptr) {
            m_target = possible;
        }
    }

    // If we don't have any visible unit to set as our target, set it to any known enemy
    // unit whatsoever.
    if (m_target == nullptr) {
        m_target = m_unitManager.enemyUnit();
    }

    // Now that we (hopefully) have a target, send all the soldiers to attack it. If we
    // don't have a target, we can't do anything.
    for (bw::Unit soldier : m_soldiers) {
        if (m_target == nullptr) {
            // If the target is invalid, then we have nowhere to send the army. So, just
            // stop each soldier entirely.
            soldier->stop();
        } else if (m_target->isVisible()) {
            // If the target unit is visible, then have the soldiers attack it directly.
            bw::Unit real = m_unitManager.getReal(m_target);
            if (soldier->getTarget() != real) {
                soldier->attack(real);
            }
        } else {
            // Otherwise, send the soldiers to attack the last known position of the unit.
            if (soldier->getTargetPosition() != m_target->getPosition()) {
                soldier->attack(m_target->getPosition());
            }
        }
    }
}

void CombatManager::onUnitDestroy(bw::Unit unit) {
    m_soldiers.erase(unit);
}