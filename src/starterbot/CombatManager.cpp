#include "CombatManager.h"

CombatManager::CombatManager(UnitManager& unitManager, IntelManager& intelManager) :
    m_unitManager(unitManager),
    m_intelManager(intelManager) {
}

void CombatManager::attack() {
    m_attacking = true;
}

void CombatManager::onStart() {
    m_attacking = false;
    m_soldiers.clear();

    m_targetPos = bw::TilePositions::Invalid;
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

    // Search for any visible enemy unit.
    bw::Unit found = m_intelManager.findUnit(nullptr);

    if (found == nullptr) {
        // If we didn't find any visible units, move to the last known position of an
        // arbitrary enemy unit. If we can't find any such units, we can't do anything.
        bw::Unit target = m_intelManager.peekUnit([&](bw::Unit unit) {
            return m_intelManager.getLastPosition(unit) != bw::TilePositions::Unknown;
        });

        m_targetPos = target == nullptr ?
            bw::TilePositions::Invalid : m_intelManager.getLastPosition(target);
        m_target = nullptr;
    } else if (m_target == nullptr) {
        // If we don't currently have a target, set this visible unit as our target.
        m_targetPos = bw::TilePositions::None;
        m_target = found;
    }

    // Now that we (hopefully) have a target, send all the soldiers to attack it.
    for (bw::Unit soldier : m_soldiers) {
        if (m_targetPos == bw::TilePositions::Invalid) {
            // If the target is invalid, then we have nowhere to send the army. So, just
            // stop the soldier entirely.
            soldier->stop();
        } else if (m_targetPos != bw::TilePositions::None &&
                bw::TilePosition(soldier->getTargetPosition()) != m_targetPos) {
            // If we do have a target position and this soldier isn't currently headed
            // there, then send the soldier to that location.
            soldier->move(bw::Position(m_targetPos));
        } else if (m_target != nullptr && soldier->getTarget() != m_target) {
            // If we have a target unit and this soldier isn't attacking it, then tell the
            // soldier to attack it.
            soldier->attack(m_target);
        }
    }
}

void CombatManager::onUnitDestroy(bw::Unit unit) {
    m_soldiers.erase(unit);

    // If the target unit we were trying to destroy was the unit that was destroyed, then
    // it's no longer our target.
    if (m_target == unit) {
        m_target = nullptr;
    }
}