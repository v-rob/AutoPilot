#include "IntelManager.h"

#include "UnitManager.h"

bool IntelManager::isEnemy(bw::Unit unit) {
    return g_game->enemies().count(unit->getPlayer());
}

bw::TilePosition IntelManager::getLastPosition(bw::Unit unit) {
    if (m_lastPositions.count(unit)) {
        return m_lastPositions[unit];
    }
    return bw::TilePositions::Unknown;
}

bw::Unit IntelManager::peekUnit(const bw::UnitFilter& pred) {
    return UnitManager::matchUnit(m_enemyUnits, pred);
}

bw::Unitset IntelManager::peekUnits(const bw::UnitFilter& pred, int count) {
    return UnitManager::matchUnits(m_enemyUnits, pred, count);
}

int IntelManager::peekCount(const bw::UnitFilter& pred) {
    return UnitManager::matchCount(m_enemyUnits, pred);
}

bw::Unit IntelManager::findUnit(const bw::UnitFilter& pred) {
    return UnitManager::matchUnit(m_visibleUnits, pred);
}

bw::Unitset IntelManager::findUnits(const bw::UnitFilter& pred, int count) {
    return UnitManager::matchUnits(m_visibleUnits, pred, count);
}

int IntelManager::findCount(const bw::UnitFilter& pred) {
    return UnitManager::matchCount(m_visibleUnits, pred);
}

void IntelManager::onStart() {
    m_enemyUnits.clear();
    m_visibleUnits.clear();

    m_lastPositions.clear();
}

void IntelManager::onFrame() {
    // Loop through all the units in the game. If the unit is known to be an enemy (which
    // can only be confirmed if the unit is currently visible), then add it to our lists
    // of units and record its current position as the last known position.
    for (bw::Unit unit : g_game->getAllUnits()) {
        if (isEnemy(unit)) {
            m_enemyUnits.insert(unit);
            m_visibleUnits.insert(unit);

            m_lastPositions[unit] = unit->getTilePosition();
        }
    }

    // Loop through all known last positions. If the unit is not currently visible, but
    // the last known position currently is visible, then the unit must have moved. So,
    // remove its position information.
    for (auto it = m_lastPositions.begin(); it != m_lastPositions.end();) {
        if (!it->first->isVisible() && g_game->isVisible(it->second)) {
            it = m_lastPositions.erase(it);
        } else {
            ++it;
        }
    }
}

void IntelManager::onUnitHide(bw::Unit unit) {
    m_visibleUnits.erase(unit);
}

void IntelManager::onUnitDestroy(bw::Unit unit) {
    m_enemyUnits.erase(unit);
    m_visibleUnits.erase(unit);

    m_lastPositions.erase(unit);
}