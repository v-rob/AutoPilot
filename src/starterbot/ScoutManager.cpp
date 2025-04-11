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
            break;
        }
    }

    // If position is not our base, add it to the list of startingLocations the Scout should search.
    for (bw::TilePosition spawnPos : g_game->getStartLocations()) {
        if (spawnPos.x != myBase.x and spawnPos.y != myBase.y){
            startingLocations.push_back(spawnPos);
            std::cout << spawnPos << ""; 
        }
    }

    std::cout << "\n " << "";
}

void ScoutManager::onFrame() {
    /* 
    *  For now, scouting behavior is more complex. The scout will go through all spawn locations
    *  besides our location. When it finds the enemy base, use VectorField to retreive
    *  the direction of the vector at the Scout's current Position. The scout's movement is
    *  dependent on where the Vector is pointing at the specific Scout Position.
    */


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