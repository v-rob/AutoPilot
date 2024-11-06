#include "UnitManager.h"

bw::Unit UnitManager::matchUnit(const bw::Unitset& units, const bw::UnitFilter& pred) {
    // Iterate through the set of units and return the first one that matches.
    for (bw::Unit unit : units) {
        if (!pred.isValid() || pred(unit)) {
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

bw::Unit UnitManager::peekUnit(const bw::UnitFilter& pred, bool progress) {
    return matchUnit(progress ? g_self->getUnits() : m_allUnits, pred);
}

bw::Unitset UnitManager::peekUnits(const bw::UnitFilter& pred, int count, bool progress) {
    return matchUnits(progress ? g_self->getUnits() : m_allUnits, pred, count);
}

int UnitManager::peekCount(const bw::UnitFilter& pred, bool progress) {
    return matchCount(progress ? g_self->getUnits() : m_allUnits, pred);
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
    m_allUnits.clear();
    m_freeUnits.clear();
}

void UnitManager::onUnitComplete(bw::Unit unit) {
    m_allUnits.insert(unit);
    m_freeUnits.insert(unit);
}

void UnitManager::onUnitDestroy(bw::Unit unit) {
    m_allUnits.erase(unit);
    m_freeUnits.erase(unit);
}