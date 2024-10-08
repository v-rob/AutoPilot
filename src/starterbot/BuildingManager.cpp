#include "BuildingManager.h"

BuildingManager::BuildingManager(UnitManager& unitManager) :
    m_unitManager(unitManager) {
}

bool BuildingManager::addTrainRequest(bw::UnitType type) {
    bw::Unit building = m_unitManager.reserveUnit(bw::Filter::GetType == type.whatBuilds().first);
    if (building == nullptr) {
        return false;
    }

    if (!building->train(type)) {
        m_unitManager.releaseUnit(building);
        return false;
    }

    m_buildings.insert(building);
    return true;
}

void BuildingManager::onStart() {
    m_buildings.clear();
}

void BuildingManager::onFrame() {
    m_unitManager.releaseUnits(m_buildings, !bw::Filter::IsTraining);
}

void BuildingManager::onUnitDestroy(bw::Unit unit) {
    m_buildings.erase(unit);
}