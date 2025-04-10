#include "CombatManager.h"

CombatManager::CombatManager(UnitManager& unitManager) :
    m_unitManager(unitManager) {
}

void CombatManager::startAttack() {
    // If we aren't already attacking, we need to set everything up.
    if (!m_isAttacking) {
        // Move all our defensive units into the offensive set, and clear out the set of
        // defensive units. The offensive leader gets decided later.
        m_offenseUnits.insert(m_defenseUnits.begin(), m_defenseUnits.end());
        m_offenseLeader = nullptr;

        m_defenseUnits.clear();
        m_defenseLeader = nullptr;

        // Since units have moved into a different set, we need to recluster everything.
        m_clusterTimer = 0;

        // We want to get the army to the enemy base rather than attacking just yet.
        m_isWaiting = true;
    }

    m_isAttacking = true;
}

void CombatManager::onStart() {
    m_clusterTimer = 0;
    m_defenseTimer = 0;
    m_offenseTimer = 0;

    m_isDefending = false;
    m_isAttacking = false;
    m_isWaiting = false;

    m_defenseUnits.clear();
    m_offenseUnits.clear();
    m_dangerUnits.clear();

    m_defenseClusters.clear();
    m_offenseClusters.clear();
    m_dangerClusters.clear();

    m_defenseLeader = nullptr;
    m_offenseLeader = nullptr;
}

void CombatManager::onFrame() {
    updateUnits();
    updateDefense();
    updateOffense();
}

void CombatManager::onDraw() {
    drawClusters(m_defenseClusters, WAIT_RADIUS, bw::Colors::Purple);
    drawClusters(m_offenseClusters, WAIT_RADIUS, bw::Colors::Purple);
    drawClusters(m_dangerClusters, DANGER_RADIUS, bw::Colors::Orange);
}

void CombatManager::onUnitDestroy(bw::Unit unit) {
    m_defenseUnits.erase(unit);
    m_offenseUnits.erase(unit);

    // If the unit is the defensive or offensive leader, just set it to null. The relevant
    // functions will decide which unit to assign as the leader later on.
    if (m_defenseLeader == unit) {
        m_defenseLeader = nullptr;
    }
    if (m_offenseLeader == unit) {
        m_offenseLeader = nullptr;
    }

    // Since a unit was destroyed, we need to recompute the clusters to avoid having any
    // dead units in the clusters.
    m_clusterTimer = 0;
}

void CombatManager::updateUnits() {
    // If we have any non-worker non-building units that can attack, reserve them and add
    // them to our set of defensive units.
    bw::Unitset reserved =
        m_unitManager.reserveUnits(bw::CanAttack && bw::CanMove && !bw::IsWorker);
    m_defenseUnits.insert(reserved.begin(), reserved.end());

    // We find all known enemy units with the same criteria, putting them in the set of
    // dangerous enemy units.
    m_dangerUnits = m_unitManager.enemyUnits(bw::CanAttack && bw::CanMove && !bw::IsWorker);

    // If our recluster timeout has occurred or we obtained some new defensive units, we
    // need to recompute the unit clusters.
    if (m_clusterTimer == 0 || !reserved.empty()) {
        m_clusterTimer = RETARGET_TIME;

        m_defenseClusters = findUnitClusters(m_defenseUnits, IDEAL_CLUSTER, MAX_CLUSTER);
        m_offenseClusters = findUnitClusters(m_offenseUnits, IDEAL_CLUSTER, MAX_CLUSTER);
        m_dangerClusters = findUnitClusters(m_dangerUnits, IDEAL_CLUSTER, MAX_CLUSTER);
    } else {
        m_clusterTimer--;
    }
}

void CombatManager::updateDefense() {
    // If there are no defensive units, we can't do any defensive stuff.
    if (m_defenseUnits.empty()) {
        return;
    }

    // Check if there are any visible enemies within the danger radius of our base units.
    // If so, we need to have our defensive code kick in.
    bool defend = false;

    for (bw::Unit baseUnit : getBaseUnits(g_self)) {
        if (hasUnitInRadius(m_dangerUnits, baseUnit->getPosition(), DANGER_RADIUS)) {
            defend = true;
            break;
        }
    }

    // If we need to defend and weren't doing so before, reset the defense timer so our
    // units can start defending right away.
    if (defend && !m_isDefending) {
        m_defenseTimer = 0;
    }
    m_isDefending = defend;

    // Now that we know our current defensive status, we can decide what our defensive
    // units should be doing.
    if (m_isDefending) {
        updateActiveDefense();
    } else {
        updatePassiveDefense();
    }
}

void CombatManager::updatePassiveDefense() {
    // If we have any defensive units whatsoever but no leader, choose a completely
    // arbitrary one as the leader. We just need a unit to group around.
    if (m_defenseLeader == nullptr) {
        m_defenseLeader = *m_defenseUnits.begin();
    }

    // If we're not currently defending the base, we want to move our units roughly
    // towards the opening leading to our base to be ready for an attack.
    bw::Unitset buildings = m_unitManager.selfUnits(bw::IsBuilding);
    bw::Position center(bw::TilePosition(g_game->mapWidth() / 2, g_game->mapHeight() / 2));

    // Send the defensive leader towards the center of the map (which will force it
    // through or near the base's pinch point) until it gets far enough away from the
    // base, at which point we can stop moving.
    if (hasUnitInRadius(buildings, m_defenseLeader->getPosition(), GROUP_RADIUS)) {
        m_defenseLeader->move(center);
    } else if (m_defenseLeader->isMoving()) {
        m_defenseLeader->stop();
    }

    // All the rest of the defensive units should just follow the defensive leader,
    // resulting in a tight group of units ready to defend the base,
    for (bw::Unit unit : m_defenseUnits) {
        if (unit == m_defenseLeader) {
            continue;
        }

        unit->follow(m_defenseLeader);
    }
}

void CombatManager::updateActiveDefense() {
    // Update our retarget timeout. If it hasn't expired yet, we don't do any retargeting.
    if (m_defenseTimer != 0) {
        m_defenseTimer--;
        return;
    }
    m_defenseTimer = RETARGET_TIME;

    // We need to choose a target for each cluster of defensive units. The simplest
    // choice, that of choosing the closest dangerous enemy unit, is simple but gives
    // fairly good results.
    for (Cluster& cluster : m_defenseClusters) {
        bw::Unit target = getClosestUnit(m_dangerUnits, cluster.centroid);

        for (bw::Unit unit : cluster.units) {
            unit->attack(m_unitManager.getReal(target));
        }
    }
}

void CombatManager::updateOffense() {
    // If we don't have any offensive units, then we call off any current attacks.
    if (m_offenseUnits.empty()) {
        m_isAttacking = false;
        return;
    }

    // We decide what to do with our units based on whether the units are moving to the
    // enemy base or are currently attacking.
    if (m_isWaiting) {
        updateWaitingOffense();
    } else {
        updateAttackOffense();
    }
}

void CombatManager::updateWaitingOffense() {
    if (m_offenseLeader == nullptr) {
        // If we don't currently have an offensive leader, then all of our units still
        // need to get close to the enemy base.
        bw::Unitset buildings = m_unitManager.enemyUnits(bw::IsBuilding);
        bw::Unitset baseUnits = getBaseUnits(g_game->enemy());

        for (bw::Unit unit : m_offenseUnits) {
            // Tell each unit to move towards the closest known building at the enemy base
            // to get them moving over there.
            bw::Unit target = getClosestUnit(buildings, unit->getPosition());
            if (target != nullptr) {
                unit->move(target->getPosition());
            }

            // If any unit gets within the waiting radius of the enemy base, stop it and
            // set it as the offensive leader so the rest of the army can regroup.
            if (hasUnitInRadius(baseUnits, unit->getPosition(), WAIT_RADIUS)) {
                m_offenseLeader = unit;
                unit->stop();
                break;
            }
        }
    } else {
        // If we have an offensive leader, we need to finish regrouping the units before
        // we can start attacking.
        bool fullGroup = true;

        for (bw::Unit unit : m_offenseUnits) {
            // Have each unit follow the offensive leader (except for the leader itself)
            // so they can all regroup.
            if (unit == m_offenseLeader) {
                continue;
            }

            unit->follow(m_offenseLeader);

            // If there are any units that aren't within the regrouping radius of the
            // defensive leader, then we aren't fully regrouped yet.
            if (!isInRadius(unit->getPosition(), m_offenseLeader->getPosition(), GROUP_RADIUS)) {
                fullGroup = false;
            }
        }

        // If we are fully regrouped, we can stop waiting and reset the offensive timer so
        // we can begin the battle!
        if (fullGroup) {
            m_isWaiting = false;
            m_offenseTimer = 0;
        }
    }

    // If, at any point during our travel to the enemy base or regrouping procedure,
    // dangerous enemy units get too close to any of our attacking units, we can't take
    // the time to regroup and must begin attacking.
    for (bw::Unit unit : m_offenseUnits) {
        if (hasUnitInRadius(m_dangerUnits, unit->getPosition(), DANGER_RADIUS)) {
            m_isWaiting = false;
            m_offenseTimer = 0;
            break;
        }
    }
}

void CombatManager::updateAttackOffense() {
    // Update our retarget timeout. If it hasn't expired yet, we don't do any retargeting.
    if (m_offenseTimer != 0) {
        m_offenseTimer--;
        return;
    }
    m_offenseTimer = RETARGET_TIME;

    // Attacking code is slighly more complex than defensive code, since we need to attack
    // the enemy's workers and buildings as well as their troops. However, we still choose
    // which one to attack based on their proximity.
    bw::Unitset enemyAttack = m_unitManager.enemyUnits(bw::CanAttack);
    bw::Unitset enemyOthers = m_unitManager.enemyUnits(bw::IsTargetable);

    for (Cluster& cluster : m_offenseClusters) {
        // Choose the target to attack as follows: first, attack dangerous troops, as they
        // are the most likely to kill our soldiers. Second, attack anything else that can
        // attack us, namely certain buildings and workers, which also cuts off their
        // means of production. Finally, attack anything else that can be attacked.
        bw::Unit target = getClosestUnit(m_dangerUnits, cluster.centroid);
        if (target == nullptr) {
            target = getClosestUnit(enemyAttack, cluster.centroid);
        }
        if (target == nullptr) {
            target = getClosestUnit(enemyOthers, cluster.centroid);
        }

        for (bw::Unit unit : cluster.units) {
            unit->attack(m_unitManager.getReal(target));
        }
    }
}

bw::Unitset CombatManager::getBaseUnits(bw::Player player) {
    // Start with the set of buildings owned by this player.
    bw::Unitset buildings = m_unitManager.shadowUnits(bw::GetPlayer == player && bw::IsBuilding);
    bw::Unitset baseUnits;

    for (bw::Unit building : buildings) {
        // Add all the units owned by this player that are within the base radius of this
        // building to the set.
        bw::Unitset nearUnits = getUnitsInRadius(m_unitManager.shadowUnits(bw::GetPlayer == player),
            building->getPosition(), BASE_RADIUS);
        baseUnits.insert(nearUnits.begin(), nearUnits.end());
    }

    return baseUnits;
}

void CombatManager::drawClusters(
        const std::vector<Cluster>& clusters, int radius, bw::Color color) {
    for (const Cluster& cluster : clusters) {
        // Don't bother drawing a cluster that is empty to avoid useless circles drawn
        // around false centroids of empty sets.
        if (cluster.units.empty()) {
            continue;
        }

        // Draw a line between every pair of units in this cluster with the appropriate
        // color, which is a crude but effective way to delineate the units in a cluster.
        for (bw::Unit first : cluster.units) {
            for (bw::Unit second : cluster.units) {
                g_game->drawLineMap(first->getPosition(), second->getPosition(), color);
            }
        }

        // Draw a dot at the centroid of the cluster and a circle at the specified radius.
        g_game->drawCircleMap(cluster.centroid, 2, color, true);
        g_game->drawCircleMap(cluster.centroid, radius, color);
    }
}