#include "CombatManager.h"

CombatManager::CombatManager(UnitManager& unitManager) :
    m_unitManager(unitManager) {
}

void CombatManager::attack() {
    m_attacking = true;
}

static double calcBattleHeuristic(const Cluster& soldiers, const Cluster& targets) {
    double goodness = 0.0;
    int us_army = 0;
    int enemy_army = 0;

    for (bw::Unit soldier : soldiers.units) {
        for (bw::Unit target : targets.units) {
            goodness += soldier->getHitPoints() + soldier->getShields();
            goodness -= target->getHitPoints() + target->getShields();
        }
    }

    goodness -= soldiers.centroid.getDistance(targets.centroid);

    return goodness;
}

double CombatManager::pairwise(bw::Unit& attacker, bw::Unit& defender) {

    // Type of weapon being used, could be air or ground
    bw::WeaponType weapon = NULL;

    // Damage per second, to be evaluated after finding weapon type
    double dps = 0;

    // Gets either air or ground weapon
    if (attacker->getType().airWeapon() != NULL) {
        weapon = attacker->getType().airWeapon();

        // Calculate DPS for air hitting weapon
        dps = weapon.damageAmount() * weapon.damageFactor() * attacker->getType().maxAirHits()
            - defender->getType().armor() / weapon.damageCooldown();
    }
    else {
        weapon = attacker->getType().groundWeapon();

        // Calculate DPS for ground hitting weapon
        dps = weapon.damageAmount() * weapon.damageFactor() * attacker->getType().maxGroundHits()
            - defender->getType().armor() / weapon.damageCooldown();
    }

    // if defender is a flyer and attacker cannot target defender (only targets ground)
    if (defender->getType().isFlyer() && (weapon.targetsGround()))
    {
        return 0;
    }

    // if defender is a ground unit and attacker cannot target defender (only targets air)
    if (!(defender->getType().isFlyer()) && (weapon.targetsAir())) {
        return 0;
    }

    // DMG, damage that can be dealt after x amount of frames (*** TBD ***)
    double dmg = dps * m_retargetTime;

    // Ending calculation of hitpoints + shields + damage (*** MODIFIER HERE ***)
    return (defender->getType().maxHitPoints() - defender->getHitPoints()) +
           (defender->getType().maxShields() - defender->getShields()) + 
            dmg;
}

double CombatManager::group(const Cluster& soldiers, const Cluster& targets) {
    double goodness = 0.0;
    int us_army = 0;
    int enemy_army = 0;


    // Count our soldiers
    for (bw::Unit soldier : soldiers.units) {
        us_army += 1;
    }
    // Count enemy targers
    for (bw::Unit target : targets.units) {
        enemy_army += 1;
    }

    for (bw::Unit soldier : soldiers.units) {
        for (bw::Unit target : targets.units) {

            goodness += pairwise(soldier, target) - pairwise(target, soldier);
        }
    }

    // goodness divided by the total units of us + them
    goodness = goodness / (us_army + enemy_army);

    // ***STILL NEED TO ADD LOG(DISTANCE)***
    // g-= log( us.distance(them) / avgspeed(us))
    
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

            double goodness = group(soldiers, enemies);

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