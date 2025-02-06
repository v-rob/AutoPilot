#include "CombatManager.h"

#include "UnitTools.h"

CombatManager::CombatManager(UnitManager& unitManager) :
    m_unitManager(unitManager) {
}

void CombatManager::attack() {
    m_attacking = true;
}

static double calcBattleHeuristic(const Cluster& soldiers, const Cluster& targets) {
    double goodness = 0.0;

    for (bw::Unit soldier : soldiers.units) {
        for (bw::Unit target : targets.units) {
            goodness += soldier->getHitPoints() + soldier->getShields();
            goodness -= target->getHitPoints() + target->getShields();
        }
    }

    goodness -= soldiers.centroid.getDistance(targets.centroid);

    return goodness;
}

void CombatManager::onStart() {
    m_attacking = false;
    m_retargetTime = 0;
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

    if (m_retargetTime != 0) {
        m_retargetTime--;
        return;
    }

    m_retargetTime = 300;

    std::vector<Cluster> soldierClusters = findUnitClusters(m_soldiers, 7, 10);
    std::vector<Cluster> targetClusters = findUnitClusters(m_unitManager.enemyUnits(), 7, 10);

    for (Cluster& soldiers : soldierClusters) {
        if (soldiers.units.empty()) {
            continue;
        }

        Cluster* bestTarget = nullptr;
        double bestGoodness = -INFINITY;

        for (Cluster& enemies : targetClusters) {
            if (enemies.units.empty()) {
                continue;
            }

            double goodness = calcBattleHeuristic(soldiers, enemies);

            if (goodness > bestGoodness) {
                bestTarget = &enemies;
                bestGoodness = goodness;
            }
        }

        if (bestTarget != nullptr) {
            printf("%f\n", bestGoodness);
            for (bw::Unit soldier : soldiers.units) {
                soldier->attack(bestTarget->centroid);
            }
        }
    }
}

void CombatManager::onDraw() {
    std::vector<Cluster> clusters = findUnitClusters(m_unitManager.enemyUnits(), 7, 10);

    for (Cluster& cluster : clusters) {
        for (bw::Unit first : cluster.units) {
            for (bw::Unit second : cluster.units) {
                g_game->drawLineMap(first->getPosition(), second->getPosition(), bw::Colors::Orange);
            }
        }
    }
}

void CombatManager::onUnitDestroy(bw::Unit unit) {
    m_soldiers.erase(unit);
}