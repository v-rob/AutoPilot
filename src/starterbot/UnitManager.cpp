#include "UnitManager.h"

ShadowUnit UnitManager::getShadow(bw::Unit unit) {
    // If we already have a shadow unit corresponding to this unit ID, return it.
    auto it = m_shadowMap.find(unit->getID());
    if (it != m_shadowMap.end()) {
        return &it->second;
    }

    // Otherwise, create a new shadow unit object and insert it into the map and set.
    ShadowUnit shadow = &m_shadowMap.emplace(unit->getID(), ShadowUnitImpl(unit)).first->second;
    m_shadowUnits.insert(shadow);

    // If the unit is owned by the bot or the enemy, add it to the corresponding set.
    if (shadow->getPlayer() == g_self) {
        m_selfUnits.insert(shadow);
    } else if (shadow->getPlayer() == g_game->enemy()) {
        m_enemyUnits.insert(shadow);
    }

    return shadow;
}

bw::Unit UnitManager::getReal(bw::Unit unit) {
    return getShadow(unit)->getRealUnit();
}

bool UnitManager::isAlive(bw::Unit unit) {
    return m_shadowUnits.contains(getShadow(unit));
}

bw::Unit UnitManager::matchUnit(const bw::Unitset& units, const bw::UnitFilter& pred) {
    // Iterate through the set of units and return the first one that matches.
    for (bw::Unit unit : units) {
        if((!pred.isValid() || pred(unit)) and (g_game->isVisible(unit->getTilePosition()))){
            return unit;
        }
    }

    return nullptr;
}

bw::Unitset UnitManager::matchUnits(const bw::Unitset& units, const bw::UnitFilter& pred, int count) {
    bw::Unitset matches;

    for (bw::Unit unit : units) {
        // If we've already hit the maximum number of units to be returned, exit the loop.
        if (matches.size() >= count) {
            break;
        }

        // Otherwise, add this unit to the matching set if it matches the predicate.
        if (!pred.isValid() || pred(unit)) {
            matches.insert(unit);
        }
    }

    return matches;
}

int UnitManager::matchCount(const bw::Unitset& units, const bw::UnitFilter& pred) {
    int count = 0;

    // Iterate through the set of units and increment the count for each matching unit.
    for (bw::Unit unit : units) {
        if (!pred.isValid() || pred(unit)) {
            count++;
        }
    }

    return count;
}

bw::Unit UnitManager::shadowUnit(const bw::UnitFilter& pred) {
    return matchUnit(m_shadowUnits, pred);
}

bw::Unitset UnitManager::shadowUnits(const bw::UnitFilter& pred, int count) {
    return matchUnits(m_shadowUnits, pred, count);
}

int UnitManager::shadowCount(const bw::UnitFilter& pred) {
    return matchCount(m_shadowUnits, pred);
}

bw::Unit UnitManager::selfUnit(const bw::UnitFilter& pred) {
    return matchUnit(m_selfUnits, pred);
}

bw::Unitset UnitManager::selfUnits(const bw::UnitFilter& pred, int count) {
    return matchUnits(m_selfUnits, pred, count);
}

int UnitManager::selfCount(const bw::UnitFilter& pred) {
    return matchCount(m_selfUnits, pred);
}

bw::Unit UnitManager::enemyUnit(const bw::UnitFilter& pred) {
    return matchUnit(m_enemyUnits, pred);
}

bw::Unitset UnitManager::enemyUnits(const bw::UnitFilter& pred, int count) {
    return matchUnits(m_enemyUnits, pred, count);
}

int UnitManager::enemyCount(const bw::UnitFilter& pred) {
    return matchCount(m_enemyUnits, pred);
}

bw::Unit UnitManager::borrowUnit(const bw::UnitFilter& pred) {
    return matchUnit(m_freeUnits, pred);
}

bw::Unitset UnitManager::borrowUnits(const bw::UnitFilter& pred, int count) {
    return matchUnits(m_freeUnits, pred, count);
}

int UnitManager::borrowCount(const bw::UnitFilter& pred) {
    return matchCount(m_freeUnits, pred);
}

bw::Unit UnitManager::reserveUnit(const bw::UnitFilter& pred) {
    // Try to find a free unit that matches the predicate.
    bw::Unit unit = matchUnit(m_freeUnits, pred);

    // If we found a suitable match, remove it from the set of free units.
    if (unit != nullptr) {
        m_freeUnits.erase(unit);
    }

    return unit;
}

bw::Unitset UnitManager::reserveUnits(const bw::UnitFilter& pred, int count) {
    // Find all free units that match the predicate up to the maximum.
    bw::Unitset units = matchUnits(m_freeUnits, pred, count);

    // Remove each matched unit from the set of free units.
    for (bw::Unit unit : units) {
        m_freeUnits.erase(unit);
    }

    return units;
}

void UnitManager::releaseUnit(bw::Unit& unit, const bw::UnitFilter& pred) {
    // If the unit to be potentially released matches the predicate, add it back to the
    // set of free units and set the reference to the released unit to nullptr.
    if (!pred.isValid() || pred(unit)) {
        m_freeUnits.insert(unit);
        unit = nullptr;
    }
}

void UnitManager::releaseUnits(bw::Unitset& units, const bw::UnitFilter& pred) {
    // Iterate through the set of units to be potentially released.
    for (auto it = units.begin(); it != units.end();) {
        bw::Unit unit = *it;

        // If the unit matches the predicate, add it back to the set of free units and
        // remove the now-released unit from the set. We have to update the iterator here
        // since iterators to erased elements are invalidated.
        if (!pred.isValid() || pred(unit)) {
            m_freeUnits.insert(unit);
            it = units.erase(it);
        } else {
            ++it;
        }
    }
}

void UnitManager::onStart() {
    // Now that a new game has started, we need to clear out all the old units.
    m_shadowMap.clear();

    m_shadowUnits.clear();
    m_selfUnits.clear();
    m_enemyUnits.clear();

    m_freeUnits.clear();

    // When the game starts, create shadow units for every unit that is initially known to
    // exist in the game.
    for (bw::Unit unit : g_game->getStaticNeutralUnits()) {
        getShadow(unit);
    }
}

void UnitManager::onFrame() {
    // Every frame, create a shadow unit for any units that don't currently have one.
    for (bw::Player player : g_game->getPlayers()) {
        for (bw::Unit unit : player->getUnits()) {
            getShadow(unit);
        }
    }

    // Update the fields for every shadow unit that is currently visible.
    for (auto& entry : m_shadowMap) {
        entry.second.updateFields();
    }
}

void UnitManager::onUnitComplete(bw::Unit unit) {
    // When a unit owned by the player becomes complete, it is available to be reserved.
    if (unit->getPlayer() == g_self) {
        m_freeUnits.insert(unit);
    }
}

void UnitManager::onUnitDestroy(bw::Unit unit) {
    // Erase the unit from the various sets it may be in, but don't erase it from
    // m_shadowMap because we want pointers to the shadow unit to stay valid.
    bw::Unit shadow = getShadow(unit);

    m_shadowUnits.erase(shadow);
    m_selfUnits.erase(shadow);
    m_enemyUnits.erase(shadow);

    m_freeUnits.erase(unit);
}