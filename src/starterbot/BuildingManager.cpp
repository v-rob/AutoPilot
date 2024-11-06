#include "BuildingManager.h"

BuildingManager::BuildingManager(UnitManager& unitManager) :
    m_unitManager(unitManager) {
}

bool BuildingManager::addTrainRequest(bw::UnitType type) {
    // Try to reserve a building of the appropriate type, if we have one.
    bw::Unit building = m_unitManager.reserveUnit(bw::Filter::GetType == type.whatBuilds().first);
    if (building == nullptr) {
        return false;
    }

    // Instruct the building to train the new unit. If this fails, such as due to
    // insufficient resources, release the unit and return false.
    if (!building->train(type)) {
        m_unitManager.releaseUnit(building);
        return false;
    }

    // The train request succeeded, so add this building to the set of reserved buildings.
    m_buildings.insert(building);
    return true;
}

void BuildingManager::onStart() {
    m_buildings.clear();
}

void BuildingManager::onFrame() {
    // If we have any buildings that aren't currently training any units, release them.
    m_unitManager.releaseUnits(m_buildings, !bw::Filter::IsTraining);
}

void BuildingManager::onUnitDestroy(bw::Unit unit) {
    m_buildings.erase(unit);
}