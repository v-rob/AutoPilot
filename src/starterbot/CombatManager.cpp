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

    m_targetPos = g_self->getStartLocation();
    m_target = nullptr;
}

void CombatManager::onFrame() {
    if (!m_attacking) {
        return;
    }

    bw::Unitset newSoldiers = m_unitManager.reserveUnits(bw::Filter::CanAttack);
    m_soldiers.insert(newSoldiers.begin(), newSoldiers.end());

    bw::Unit found = m_intelManager.findUnit(nullptr);

    if (found == nullptr) {
        bw::Unit target = m_intelManager.peekUnit([&](bw::Unit unit) {
            return m_intelManager.getLastPosition(unit) != bw::TilePositions::Unknown;
        });

        m_targetPos = target == nullptr ?
            bw::TilePositions::Invalid : m_intelManager.getLastPosition(target);
        m_target = nullptr;
    } else if (m_target == nullptr) {
        m_targetPos = bw::TilePositions::None;
        m_target = found;
    }

    for (bw::Unit soldier : m_soldiers) {
        if (m_targetPos == bw::TilePositions::Invalid) {
            soldier->stop();
        } else if (m_targetPos != bw::TilePositions::None &&
                bw::TilePosition(soldier->getTargetPosition()) != m_targetPos) {
            soldier->move(bw::Position(m_targetPos));
        } else if (m_target != nullptr && soldier->getTarget() != m_target) {
            soldier->attack(m_target);
        }
    }
}

void CombatManager::onUnitDestroy(bw::Unit unit) {
    m_soldiers.erase(unit);

    if (m_target == unit) {
        m_target = nullptr;
    }
}