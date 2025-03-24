#include "ScoutManager.h"
#include <cmath>

ScoutManager::ScoutManager(UnitManager& unitManager) :
    m_unitManager(unitManager), m_vectorField(unitManager), finishSearchEnemyBase(false),
    maneuverPathAdded(false), reachedPointOne(true), reachedPointTwo(false), reachedPointMiddle(false), goingRight(true){
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
    finishSearchEnemyBase = false;
    goingRight = true;
}

void ScoutManager::onFrame() {
    // For now, scouting behavior is very simplistic. Scouts are simply sent to each
    // unexplored potential start location in order to find the enemy's location. They
    // will not patrol the area any further after doing so.

    bw::TilePosition firstPos;
    bw::TilePosition secondPos;
    bw::TilePosition middlePos;

    if (finishSearchEnemyBase) {
        //REPLACE THIS WITH THE POINTS FROM THE VECTOR FIELD
        firstPos = bw::TilePosition(enemyBasePos.x + 6,enemyBasePos.y - 6);
        secondPos = bw::TilePosition(enemyBasePos.x - 6, enemyBasePos.y - 6);
        middlePos = bw::TilePosition(enemyBasePos.x, enemyBasePos.y - 9);
    }



    for (bw::Unit scout : m_scouts) {
        if (finishSearchEnemyBase == true) { // we will use the vectors for maneuvering
        //    for (VectorField vector : ) {
        //        scout->move(vector);
        //    }
            if (reachedPointOne) {
                scout->move(bw::Position(middlePos));
                if (std::abs(scout->getTilePosition().x - middlePos.x) <= 1 && std::abs(scout->getTilePosition().y - middlePos.y) <= 1) {
                    std::cout << "Point Middle Reached!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" << "";
                    goingRight = true;
                    reachedPointOne = false;
                    reachedPointTwo = false;
                    reachedPointMiddle = true;
                }
            }

            if (reachedPointMiddle){
                std::cout << "[" << secondPos.x << "," << secondPos.y << "]" << " SecondPos\n";  // this is correct for TilePosition
                std::cout << "[" << scout->getTilePosition().x << "," << scout->getTilePosition().y << "]" << "CurrentScout\n"; //current scout is a problem, lk tiles instead of TilePosition
                if (goingRight) {
                    scout->move(bw::Position(secondPos)); //move() takes in a Position, not TilePosition
                    if (std::abs(scout->getTilePosition().x - secondPos.x) <= 1 && std::abs(scout->getTilePosition().y - secondPos.y) <= 1) {
                        //scout has just reached to the secondPos location
                        std::cout << "Point Two Reached!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" << "";
                        reachedPointOne = false;
                        reachedPointTwo = true;
                        reachedPointMiddle = false; 
                    }
                }
                else { //going left
                    scout->move(bw::Position(firstPos));// takes in a Position, not TilePosition
                    if (std::abs(scout->getTilePosition().x - firstPos.x) <= 1 && std::abs(scout->getTilePosition().y - firstPos.y) <= 1) {
                        //scout has just reached to the secondPos location
                        std::cout << "Point One Reached!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" << "";
                        reachedPointOne = true;
                        reachedPointTwo = false;
                        reachedPointMiddle = false;
                    }
                }
            }
            if (reachedPointTwo) {
                scout->move(bw::Position(middlePos));
                std::cout << "[" << firstPos.x << "," << firstPos.y << "]" << "firstPos\n"; //This is correct TilePosition
                std::cout << "[" << scout->getTilePosition().x << "," << scout->getTilePosition().y << "]" << "current scout to pos one\n"; //current scout
                if (std::abs(scout->getTilePosition().x - middlePos.x) <= 1 && std::abs(scout->getTilePosition().y - middlePos.y) <= 1) {
                    //scout has just reached to the secondPos location
                    std::cout << "Point middle reached!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" << "";
                    reachedPointOne = false;
                    reachedPointTwo = false;
                    reachedPointMiddle = true;
                    goingRight = false;
                }
            }
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

            if ((g_game->isExplored(pos) and found != nullptr and finishSearchEnemyBase == false)
                || !g_game->isExplored(pos) and (std::abs(pos.x - scout->getPosition().x) < 3
                    and (pos.y - std::abs(scout->getPosition().y) < 3) and found == nullptr)) { 
                //|| !g_game->isExplored(pos) and (std::abs(pos.x - scout->getPosition().x) < 3
                //    and (pos.y - std::abs(scout->getPosition().y) < 3) and found == nullptr)
                
                
                //switch this back to != when done experimenting, 2nd condition for scout that can't reach
                //to position because it's stuck from moving to spawn pos due to building.
                if (finishSearchEnemyBase !=false) {
                    std::cout << "Found base" << std::endl;
                }
                enemyBasePos = pos; //TilePosition
                finishSearchEnemyBase = true;
                break;
            }
        }
    }
}

void ScoutManager::onUnitDestroy(bw::Unit unit) {
    m_scouts.erase(unit);
}