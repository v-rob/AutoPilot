#include "ScoutManager.h"
#include <cmath>

ScoutManager::ScoutManager(UnitManager& unitManager) :
    m_unitManager(unitManager), m_vectorField(unitManager), finishSearchEnemyBase(false),
    maneuverPathAdded(false){
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
    m_vectorField.notifyReceiver(event);
}

void ScoutManager::onStart() {
    m_scouts.clear();
    //maneuverPath.clear();
}

void ScoutManager::onFrame() {
    // For now, scouting behavior is very simplistic. Scouts are simply sent to each
    // unexplored potential start location in order to find the enemy's location. They
    // will not patrol the area any further after doing so.
    for (bw::Unit scout : m_scouts) {
        if (finishSearchEnemyBase) {
            break;
        }
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
            //check if scout is at an enemy spawn location
            bw::Unit found = m_unitManager.enemyUnit();

            if ((g_game->isExplored(pos) and found != nullptr)
                || !g_game->isExplored(pos) and (std::abs(pos.x - scout->getPosition().x) < 3
                    and (pos.y - std::abs(scout->getPosition().y) < 3) and found == nullptr)) { 
                //|| !g_game->isExplored(pos) and (std::abs(pos.x - scout->getPosition().x) < 3
                //    and (pos.y - std::abs(scout->getPosition().y) < 3) and found == nullptr)
                
                
                //switch this back to != when done experimenting, 2nd condition for scout that can't reach
                //to position because it's stuck from moving to spawn pos due to building.
                if (finishSearchEnemyBase == false) {
                    std::cout << "Found base" << std::endl;
                }
                enemyBasePos = pos;
                finishSearchEnemyBase = true;
                break;
            }
        }
    }

    //THIS IS COMMENTED DUE TO TESTING MANEUVER (keep it here in case)
    //if (finishSearchEnemyBase and !maneuverPathAdded) {
    //    bw::TilePosition newPos = enemyBasePos;
    //    newPos.x = enemyBasePos.x + 2; //left
    //    newPos.y = enemyBasePos.y + 2; //down
    //    maneuverPath.push_back(newPos);//bottom-left---------
    //    newPos.x = enemyBasePos.x - 2; //right
    //    newPos.y = enemyBasePos.y + 2; //down
    //    maneuverPath.push_back(newPos); //bottom-right--------
    //    newPos.x = enemyBasePos.x - 2; //right
    //    newPos.y = enemyBasePos.y - 2; //up
    //    maneuverPath.push_back(newPos); //top-right--------
    //    newPos.x = enemyBasePos.x + 2; //left
    //    newPos.y = enemyBasePos.y - 2; //up
    //    maneuverPath.push_back(newPos); //top-left--------

    //    maneuverPathAdded = true;
    //}


    //for (bw::Unit scout : m_scouts) {
    //    if (finishSearchEnemyBase == true) {
    //        for (bw::TilePosition scoutPoint : maneuverPath) {
    //            /*std::cout << "moved scout to " <<
    //                scoutPoint.x << "," <<
    //                scoutPoint.y << std::endl;*/
    //            scout->move(bw::Position(scoutPoint));
    //        }
    //    }
    //    else{
    //        break;
    //    }
    //}
}

void ScoutManager::onUnitDestroy(bw::Unit unit) {
    m_scouts.erase(unit);
}