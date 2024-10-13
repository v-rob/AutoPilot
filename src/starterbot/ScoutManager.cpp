#include "ScoutManager.h"

ScoutManager::ScoutManager(UnitManager& unitManager) :
    m_unitManager(unitManager) {
}

bool ScoutManager::addScout(bw::UnitType type) {
    bw::Unit scout = m_unitManager.reserveUnit(bw::Filter::GetType == type);
    if (scout == nullptr) {
        return false;
    }

    m_scouts.insert(scout);
    return true;
}

int ScoutManager::countScouts(bw::UnitType type) {
    return UnitManager::matchCount(m_scouts, bw::Filter::GetType == type);
}

void ScoutManager::onStart() {
    m_scouts.clear();
}

void ScoutManager::onFrame() {
    for (bw::Unit scout : m_scouts) {
        if (scout->isMoving()) {
            continue;
        }

        for (bw::TilePosition pos : g_game->getStartLocations()) {
            if (!g_game->isExplored(pos)) {
                scout->move(bw::Position(pos));
            }
        }
    }
}

void ScoutManager::onUnitDestroy(bw::Unit unit) {
    m_scouts.erase(unit);
}