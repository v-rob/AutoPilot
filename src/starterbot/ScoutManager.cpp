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
    startingLocations.clear();

    for (bw::TilePosition pos : g_game->getStartLocations()){
        std::cout << pos << "";
    }

    //first, make a list of all TilePosition Starting locations without our start location.

    // Determine our start location based on our first command center, nexus, or hatchery
    bw::TilePosition myBase = bw::TilePositions::None;

    for (const auto& unit : g_self->getUnits()) {
        if (unit->getType().isResourceDepot()) { // Command Center, Nexus, Hatchery
            myBase = unit->getTilePosition();
            break; // Found it, no need to continue
        }
    }

    std::cout << "This is where my base is at: " << myBase << "\n";


    std::cout << "And here's the list of the possible places that our scout should move: " << "";
    for (bw::TilePosition spawnPos : g_game->getStartLocations()) {
        //if the pos is not my base, add it to the starting locations that the scout should 
        if (spawnPos.x != myBase.x and spawnPos.y != myBase.y){
            startingLocations.push_back(spawnPos);
            std::cout << spawnPos << ""; 
            g_game->
        }
    }

    std::cout << "\n " << "";
}

void ScoutManager::onFrame() {
    // For now, scouting behavior is very simplistic. Scouts are simply sent to each
    // unexplored potential start location in order to find the enemy's location. They
    // will not patrol the area any further after doing so.

    bw::TilePosition firstPos;
    bw::TilePosition secondPos;
    bw::TilePosition middlePos;

    std::cout << "Finish Search Enemy Base? " << finishSearchEnemyBase << "\n"; //once when reached to spawn loc, set to 1, incorrect



    if (finishSearchEnemyBase == true){
        //REPLACE THIS WITH THE POINTS FROM THE VECTOR FIELD
      //std::cout << "[" << enemyBasePos.x << ", " << enemyBasePos.y << "]";
        if (enemyBasePos.y <= 65){//this means if the enemy base is on the top half
            firstPos = bw::TilePosition(enemyBasePos.x + 6, enemyBasePos.y +6);
            secondPos = bw::TilePosition(enemyBasePos.x - 6, enemyBasePos.y +6);
            middlePos = bw::TilePosition(enemyBasePos.x, enemyBasePos.y +9);
        }
        if (enemyBasePos.y > 65){//if enemy base is on bottom half
            firstPos = bw::TilePosition(enemyBasePos.x + 6, enemyBasePos.y - 6);
            secondPos = bw::TilePosition(enemyBasePos.x - 6, enemyBasePos.y - 6);
            middlePos = bw::TilePosition(enemyBasePos.x, enemyBasePos.y - 9);
        }
    }



    for (bw::Unit scout : m_scouts) {
        if (finishSearchEnemyBase == true) { // we will use the vectors for maneuvering

            //this is for using the scouts current position to get the new position
            //first get the current scout position
            bw::WalkPosition scoutPos = bw::WalkPosition(scout->getPosition());
            //next, get the vector of the current position that the scout is at
            std::optional<Vector2> vector = m_vectorField.getVectorSum(scoutPos.x, scoutPos.y);
            if (vector != std::nullopt) {
                bw::Position newPosition = bw::Position(scoutPos) + bw::Position((*vector) * 5);
                scout->move(bw::Position(newPosition));
            }




            //if (reachedPointOne) {
            //    scout->move(bw::Position(middlePos));
            //    if (std::abs(scout->getTilePosition().x - middlePos.x) <= 1 && std::abs(scout->getTilePosition().y - middlePos.y) <= 1) {
            //        goingRight = true;
            //        reachedPointOne = false;
            //        reachedPointTwo = false;
            //        reachedPointMiddle = true;
            //    }
            //}

            //if (reachedPointMiddle){
            //    if (goingRight) {
            //        scout->move(bw::Position(secondPos)); //move() takes in a Position, not TilePosition
            //        if (std::abs(scout->getTilePosition().x - secondPos.x) <= 1 && std::abs(scout->getTilePosition().y - secondPos.y) <= 1) {
            //            //scout has just reached to the secondPos location
            //            reachedPointOne = false;
            //            reachedPointTwo = true;
            //            reachedPointMiddle = false; 
            //        }
            //    }
            //    else { //going left
            //        scout->move(bw::Position(firstPos));// takes in a Position, not TilePosition
            //        if (std::abs(scout->getTilePosition().x - firstPos.x) <= 1 && std::abs(scout->getTilePosition().y - firstPos.y) <= 1) {
            //            //scout has just reached to the secondPos location
            //            reachedPointOne = true;
            //            reachedPointTwo = false;
            //            reachedPointMiddle = false;
            //        }
            //    }
            //}
            //if (reachedPointTwo) {
            //    scout->move(bw::Position(middlePos));
            //    if (std::abs(scout->getTilePosition().x - middlePos.x) <= 1 && std::abs(scout->getTilePosition().y - middlePos.y) <= 1) {
            //        //scout has just reached to the secondPos location
            //        reachedPointOne = false;
            //        reachedPointTwo = false;
            //        reachedPointMiddle = true;
            //        goingRight = false;
            //    }
            //}
            break;
        }
        // If the scout is currently moving towards some target location, let them move.
        if (scout->isMoving()) {
            continue;
        }

        // Otherwise, find the first potential start location that is not yet explored and
        // send the scout to explore it.


   //   for (bw::TilePosition pos : g_game->getStartLocations()) { //g_game->getStartLocations() is a std::deque<bw::TilePosition>
        for (bw::TilePosition pos : startingLocations) {
            if (!g_game->isExplored(pos)) {
                scout->move(bw::Position(pos));
                continue; //continue searching instead of breaking early
                //break;
            } //this means that spawn location is already explored
            //check if scout is at an enemy spawn location
            bw::Unit found = m_unitManager.enemyUnit();
            std::cout << found << "\n";

            if (g_game->isExplored(pos) and found != nullptr and !finishSearchEnemyBase){
                //|| !g_game->isExplored(pos) and (std::abs(pos.x - scout->getPosition().x) < 3
                //    and (pos.y - std::abs(scout->getPosition().y) < 3) and found == nullptr)) {
                if (finishSearchEnemyBase != false) {
                    std::cout << "Found base" << std::endl;
                }
                enemyBasePos = pos; //TilePosition
                finishSearchEnemyBase = true;
                break;
            }
        }

          ////this is for using the scouts current position to get the new position
          ////first get the current scout position
          //bw::WalkPosition scoutPos = bw::WalkPosition(scout->getPosition());
          ////next, get the vector of the current position that the scout is at
          //std::optional<Vector2> vector = m_vectorField.getVectorSum(scoutPos.x, scoutPos.y);
          //if (vector != std::nullopt) {
          //    bw::WalkPosition newPosition;
          //    newPosition.x = scoutPos.x + (*vector).x;
          //    newPosition.y = scoutPos.y + (*vector).y;
          //    scout->move(bw::Position(newPosition));
          //}
    }
}

void ScoutManager::onUnitDestroy(bw::Unit unit) {
    m_scouts.erase(unit);
}