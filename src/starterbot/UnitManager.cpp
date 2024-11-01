#include "UnitManager.h"

bw::Unit UnitManager::matchUnit(const bw::Unitset& units, const bw::UnitFilter& pred) {
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
        if (matches.size() >= count) {
            break;
        }

        if (!pred.isValid() || pred(unit)) {
            matches.insert(unit);
        }
    }

    return matches;
}

int UnitManager::matchCount(const bw::Unitset& units, const bw::UnitFilter& pred) {
    int count = 0;

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
    bw::Unit unit = matchUnit(m_freeUnits, pred);

    if (unit != nullptr) {
        m_freeUnits.erase(unit);
    }

    return unit;
}

bw::Unitset UnitManager::reserveUnits(const bw::UnitFilter& pred, int count) {
    bw::Unitset units = matchUnits(m_freeUnits, pred, count);

    for (bw::Unit unit : units) {
        m_freeUnits.erase(unit);
    }

    return units;
}

void UnitManager::releaseUnit(bw::Unit& unit, const bw::UnitFilter& pred) {
    if (!pred.isValid() || pred(unit)) {
        m_freeUnits.insert(unit);
        unit = nullptr;
    }
}

void UnitManager::releaseUnits(bw::Unitset& units, const bw::UnitFilter& pred) {
    for (auto it = units.begin(); it != units.end();) {
        bw::Unit unit = *it;

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