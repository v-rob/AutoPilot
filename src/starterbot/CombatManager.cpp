#include "CombatManager.h"

#include <cmath>

CombatManager::CombatManager(UnitManager& unitManager) :
    m_unitManager(unitManager) {
}

void CombatManager::attack() {
    m_attacking = true;
}

void CombatManager::onStart() {
    m_attacking = false;
    m_retargetTimer = 0;

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

    // We don't want to recompute which enemy units we're targeting every frame, since
    // that may result in the army changing targets every few frames in response to every
    // small change. So, decrement a timer and recalculate the targets when it hits zero.
    if (m_retargetTimer == 0) {
        m_retargetTimer = RETARGET_TIME;
        chooseSoldierTargets();
    } else {
        m_retargetTimer--;
    }
}

void CombatManager::onDraw() {
    drawClusters(m_soldierClusters, bw::Colors::Purple);
    drawClusters(m_enemyClusters, bw::Colors::Orange);
}

void CombatManager::onUnitDestroy(bw::Unit unit) {
    m_soldiers.erase(unit);
}

void CombatManager::chooseSoldierTargets() {
    // First, find a new clustering for our troops and the entirety of our enemy's
    // soldiers, workers, and buildings.
    m_soldierClusters = findUnitClusters(m_soldiers, IDEAL_CLUSTER, MAX_CLUSTER);
    m_enemyClusters = findUnitClusters(m_unitManager.enemyUnits(), IDEAL_CLUSTER, MAX_CLUSTER);

    // We need to choose a target for each of our clusters, which we do by calculating a
    // simple heuristic that generates a rough value of how good or bad it would be for
    // one cluster to attack another for every pairing of soldier and enemy clusters.
    for (Cluster& soldiers : m_soldierClusters) {
        // If we don't have any troops in this cluster, then there's nothing more to do.
        if (soldiers.units.empty()) {
            continue;
        }

        // Keep track of the best potential target so far based on the goodness value that
        // the heuristic function returned.
        Cluster* bestEnemy = nullptr;
        double bestGoodness = -INFINITY;

        for (Cluster& enemies : m_enemyClusters) {
            // If there are no enemies in this cluster, we don't want to target it!
            if (enemies.units.empty()) {
                continue;
            }

            // Calculate the heuristic value for this pairing of clusters. If it's better
            // than our previous best value for this cluster of soldiers, choose this
            // enemy cluster as the best pairing so far.
            double goodness = calcClusterHeuristic(soldiers, enemies);

            if (goodness > bestGoodness) {
                bestEnemy = &enemies;
                bestGoodness = goodness;
            }
        }

        // If we found an enemy cluster to target, instruct all soldiers in this cluster
        // to attack-move to the center of that cluster and hope for the best!
        if (bestEnemy != nullptr) {
            for (bw::Unit soldier : soldiers.units) {
                soldier->attack(bestEnemy->centroid);
            }
        }
    }
}

double CombatManager::calcClusterHeuristic(const Cluster& soldiers, const Cluster& enemies) {
    double goodness = 0.0;

    // For each pairing of soldier and enemy in these clusterings, compute the difference
    // in goodness between the pairwise heuristic of the battle between them and add it to
    // the overall goodness value.
    for (bw::Unit soldier : soldiers.units) {
        for (bw::Unit enemy : enemies.units) {
            goodness += calcPairwiseHeuristic(soldier, enemy) -
                calcPairwiseHeuristic(enemy, soldier);
        }
    }

    // For large clusters, the above goodness value will be inflated quadratically because
    // there are N^2 pairings between two clusters of the same size. Assuming identical
    // battles, we want small clusters to have the same heuristic value as large clusters,
    // so we divide by the number of pairings to normalize the goodness value.
    goodness /= (soldiers.units.size() * enemies.units.size());

    // Distance is factored into the goodness for an attack between clusters. First,
    // calculate the minimum speed for the cluster, since the cluster can only move as
    // fast as its slowest member.
    double minSpeed = 0.0;
    for (bw::Unit soldier : soldiers.units) {
        minSpeed = std::min(minSpeed, soldier->getType().topSpeed());
    }

    // As distance increases and/or minimum speed decreases, we want the detrimental
    // effects on the goodness value to be increased. However, since the meaningfulness of
    // distance on the heuristic value diminishes as the distance increases, take the
    // logarithm of the resulting value to account for this.
    goodness -= std::log(soldiers.centroid.getDistance(enemies.centroid) / minSpeed);

    return goodness;
}

double CombatManager::calcPairwiseHeuristic(const bw::Unit& soldier, const bw::Unit& enemy) {
    // If this soldier can't attack (such as if it is an enemy building), the heuristic
    // value is completely neutral.
    if (!soldier->canAttack()) {
        return 0.0;
    }

    bw::UnitType soldierType = soldier->getType();
    bw::UnitType enemyType = enemy->getType();

    // We need to choose which weapon to consider. If the enemy is an air unit, we need to
    // consider the air weapon; otherwise, we need to use the ground weapon.
    bw::WeaponType weapon;
    int maxHits;

    if (enemyType.isFlyer()) {
        weapon = soldierType.airWeapon();
        maxHits = soldierType.maxAirHits();
    } else {
        weapon = soldierType.groundWeapon();
        maxHits = soldierType.maxGroundHits();
    }

    // If this unit doesn't have an appropriate weapon, the heuristic is neutral again.
    if (weapon == bw::WeaponTypes::None) {
        return 0.0;
    }

    // Calculate the base damage that this soldier can do with a single hit.
    int baseDamage = weapon.damageAmount() * weapon.damageFactor() * maxHits;
    // We have to account for the enemy's armor. If this is greater than the soldier's
    // base damage, then the soldier cannot damage this unit.
    int armorDamage = std::max(baseDamage - enemyType.armor(), 0);

    // Taking into account the cooldown for this weapon gives the damage per frame.
    double damageRate = (double)armorDamage / weapon.damageCooldown();
    // Multiplying this by the number of frames between runs of the heuristic function
    // gives us the total amount of damage that this unit will do.
    double totalDamage = damageRate * RETARGET_TIME;

    // The final goodness value for this pairwise battle is proportional to how many hit
    // points/shields the enemy has lost plus the amount of damage that can be done to
    // them in the upcoming frames.
    return (enemyType.maxHitPoints() - enemy->getHitPoints()) +
        (enemyType.maxShields() - enemy->getShields()) + totalDamage;
}

void CombatManager::drawClusters(const std::vector<Cluster>& clusters, bw::Color color) {
    for (const Cluster& cluster : clusters) {
        // Draw a line between every pair of units in this cluster with the appropriate
        // color, which is a crude but effective way to delineate the units in a cluster.
        for (bw::Unit first : cluster.units) {
            for (bw::Unit second : cluster.units) {
                g_game->drawLineMap(first->getPosition(), second->getPosition(), color);
            }
        }
    }
}