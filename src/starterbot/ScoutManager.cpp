#include "ScoutManager.h"
#include <cmath>

ScoutManager::ScoutManager(UnitManager& unitManager) :
    m_unitManager(unitManager), m_vectorField(unitManager), m_baseManager(unitManager), finishSearchEnemyBase(false),
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
    m_baseManager.notifyReceiver(event);
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
            break;
        }
    }

<<<<<<< HEAD
    // If position is not our base, add it to the list of startingLocations the Scout should search.
    for (bw::TilePosition spawnPos : g_game->getStartLocations()) {
        if (spawnPos.x != myBase.x and spawnPos.y != myBase.y){
            startingLocations.push_back(spawnPos);
            std::cout << spawnPos << ""; 
        }
    }
=======
    //std::cout << "This is where my base is at: " << myBase << "\n";


    //std::cout << "And here's the list of the possible places that our scout should move: " << "";
    //for (bw::TilePosition spawnPos : g_game->getStartLocations()) {
    //    //if the pos is not my base, add it to the starting locations that the scout should 
    //    if (spawnPos.x != myBase.x and spawnPos.y != myBase.y){
    //        startingLocations.push_back(spawnPos);
    //        std::cout << spawnPos << ""; 
    //        //g_game->
    //    }
    //}
>>>>>>> 577bd5be08724953be0a5f7cc1dfee96bdf516fa

    //std::cout << "\n " << "";
}

void ScoutManager::onFrame() {
<<<<<<< HEAD
    /* 
    *  For now, scouting behavior is more complex. The scout will go through all spawn locations
    *  besides our location. When it finds the enemy base, use VectorField to retreive
    *  the direction of the vector at the Scout's current Position. The scout's movement is
    *  dependent on where the Vector is pointing at the specific Scout Position.
    */
=======
    // For now, scouting behavior is very simplistic. Scouts are simply sent to each
    // unexplored potential start location in order to find the enemy's location. They
    // will not patrol the area any further after doing so.

    bw::TilePosition firstPos;
    bw::TilePosition secondPos;
    bw::TilePosition middlePos;

    //std::cout << "Finish Search Enemy Base? " << finishSearchEnemyBase << "\n"; //once when reached to spawn loc, set to 1, incorrect



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

>>>>>>> 577bd5be08724953be0a5f7cc1dfee96bdf516fa


    for (bw::Unit scout : m_scouts) {
        // We will use the vectors for maneuvering.
        if (finishSearchEnemyBase == true) {

            //First, get the current scout position.
            bw::WalkPosition scoutPos = bw::WalkPosition(scout->getPosition());

            //Next, get the vector from the Scout's current position.
            std::optional<Vector2> vector = m_vectorField.getVectorSum(scoutPos.x, scoutPos.y);

            //A Vector can either be Null or have a value. If not null, move scout to the new position based on Vector.
            if (vector != std::nullopt) {
                bw::Position newPosition = bw::Position(scoutPos) + bw::Position((*vector) * 5);
                scout->move(bw::Position(newPosition));
            }
            break;
        }
        // If the scout is currently moving towards some target location, let them move.
        if (scout->isMoving()) {
            continue;
        }

        // Otherwise, find the first potential start location that is not yet explored and
        // send the scout to explore it.
        for (bw::TilePosition pos : startingLocations) {
            if (!g_game->isExplored(pos)) {
                scout->move(bw::Position(pos));
                continue;
            }
            //Check if scout is at an enemy spawn location.
            bw::Unit found = m_unitManager.enemyUnit();
            std::cout << found << "\n";

            if (g_game->isExplored(pos) and found != nullptr and !finishSearchEnemyBase) {
                if (finishSearchEnemyBase != false) {
                    std::cout << "Found base" << std::endl;
                }
                enemyBasePos = pos;
                finishSearchEnemyBase = true;
                break;
            }
        }
    }
}


void ScoutManager::onUnitDestroy(bw::Unit unit) {
    m_scouts.erase(unit);
}