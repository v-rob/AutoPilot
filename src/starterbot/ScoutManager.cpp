#include "ScoutManager.h"

ScoutManager::ScoutManager(UnitManager& unitManager) :
    m_unitManager(unitManager), m_baseManager(unitManager) {
}

bool ScoutManager::addScout(bw::UnitType type) {
    // Try to reserve a unit of the appropriate type, if we have one.
    bw::Unit scout = m_unitManager.reserveUnit(bw::Filter::GetType == type);
    if (scout == nullptr) {
        return false;
    }

    // If this succeeded, add it to the set of reserved scouts.
    m_scouts.insert(scout);
    return true;
}

int ScoutManager::countScouts(bw::UnitType type) {
    return UnitManager::matchCount(m_scouts, bw::Filter::GetType == type);
}

void ScoutManager::notifyMembers(const bw::Event& event) {
    m_baseManager.notifyReceiver(event);
}

void ScoutManager::onStart() {
    m_scouts.clear();
}

void ScoutManager::onFrame() {
    // For now, scouting behavior is very simplistic. Scouts are simply sent to each
    // unexplored potential start location in order to find the enemy's location. They
    // will not patrol the area any further after doing so.
    for (bw::Unit scout : m_scouts) {
        // If the scout is currently moving towards some target location, let them move.
        if (scout->isMoving()) {
            continue;
        }

        // Otherwise, find the first potential start location that is not yet explored and
        // send the scout to explore it.
        for (bw::TilePosition pos : g_game->getStartLocations()) {
            if (!g_game->isExplored(pos)) {
                scout->move(bw::Position(pos));
                break;
            }
        }
    }
}

void ScoutManager::onUnitDestroy(bw::Unit unit) {
    m_scouts.erase(unit);
}