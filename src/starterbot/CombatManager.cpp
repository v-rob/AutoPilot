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

    AttackPairs attacks = runCombatSimulation(unitListOf(m_soldiers), unitListOf(m_unitManager.enemyUnits()));

    for (int i = 0; i < attacks.self.size(); i++) {
        auto self_it = attacks.self[i]->begin();
        auto enemy_it = attacks.enemy[i]->begin();

        while (self_it != attacks.self[i]->end() && enemy_it != attacks.enemy[i]->end()) {
            (*self_it)->attack(m_unitManager.getReal(*enemy_it));

            ++self_it;
            ++enemy_it;
        }
    }
}

void CombatManager::onDraw() {
    GroupList clusters = findUnitClusters(8, unitListOf(m_unitManager.enemyUnits()));

    for (std::shared_ptr<UnitList> cluster : clusters) {
        for (bw::Unit first : *cluster) {
            for (bw::Unit second : *cluster) {
                g_game->drawLineMap(first->getPosition(), second->getPosition(), bw::Colors::Orange);
            }
        }
    }
}

void CombatManager::onUnitDestroy(bw::Unit unit) {
    m_soldiers.erase(unit);
}
