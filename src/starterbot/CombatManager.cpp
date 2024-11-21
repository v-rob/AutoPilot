#include "CombatManager.h"

#include "CombatSimulator.h"

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

    // Huzzah! The random walk way is the only way! Or not...
//    AttackList attacks = runCombatSimulation(
//        m_soldiers, m_intelManager.peekUnits(bw::Filter::IsCompleted));
//
//    for (const auto& attack : attacks) {
//        attack.first->attack(attack.second);
//    }
}

void CombatManager::onDraw() {
    bw::Unitset enemies = m_intelManager.peekUnits(bw::Filter::IsCompleted);
    std::vector<bw::Unitset> clusters = findUnitClusters(7, enemies);

    for (const bw::Unitset& cluster : clusters) {
        for (bw::Unit first : cluster) {
            for (bw::Unit second : cluster) {
                g_game->drawLineMap(first->getPosition(), second->getPosition(), bw::Colors::Orange);
            }
        }
    }
}

void CombatManager::onUnitDestroy(bw::Unit unit) {
    m_soldiers.erase(unit);
}
