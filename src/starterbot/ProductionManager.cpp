//The actual ProductionManager.cpp
#include "ProductionManager.h"


///////////////////////////////////////////////

//Constructor
/*
`ProductionManager` has a list of workers who are reserved to build buildings.
 @m_reservedWorkers - list of workers that are currently idle
*/
ProductionManager::ProductionManager(UnitManager& manager) :
	m_unitManager(manager) {
}

///////////////////////////////////////////////


//addBuildRequest
/*
When the `addBuildRequest()` function is called, the manager will attempt to
reserve a new worker and tell the worker to build the appropriate building. If
no worker could be reserved, the function returns false.
*/
bool ProductionManager::addBuildRequest(bw::UnitType buildingType) {
	bw::TilePosition pos = g_game->getBuildLocation(buildingType, g_self->getStartLocation(), 64, false);
	if (pos == bw::TilePositions::Invalid) {
		return false;
	}

	bw::Unit worker = m_unitManager.reserveUnit(bw::Filter::GetType == buildingType.whatBuilds().first);
	if (worker == nullptr) {
		return false;
	}

	if (!worker->build(buildingType, pos)) {
		m_unitManager.releaseUnit(worker);
		return false;
	}

	m_reservedWorkers.insert(worker);
	return true;
}	


//countBuildRequests
/*
The `countBuildRequests()` function counts how many buildings of the specified type
are being built by workers. (in progress builds)
*/
int ProductionManager::countBuildRequests(bw::UnitType buildingType) {
	return UnitManager::matchCount(m_reservedWorkers, bw::Filter::BuildType == buildingType || (!bw::Filter::IsConstructing && !bw::Filter::IsIdle));
}

void ProductionManager::sendIdleWorkersToMinerals() {
	for (bw::Unit unit : m_unitManager.borrowUnits(bw::Filter::IsWorker)) {
		if (unit->isIdle()) {
			bw::Unit closestMineral = g_game->getClosestUnit(bw::Position(g_self->getStartLocation()), bw::Filter::IsMineralField);

			if (closestMineral) {
				unit->gather(closestMineral);
			}
		}
	}
}

void ProductionManager::onStart() {
	m_allUnits.clear();
	m_freeUnits.clear();
}

void ProductionManager::onFrame() {
	/*printf("###\n");
	for (bw::Unit unit : m_reservedWorkers) {
		printf("%s %s %d\n", unit->getBuildType().c_str(), unit->getOrder().c_str(), unit->isIdle());
	}*/
	m_unitManager.releaseUnits(m_reservedWorkers, bw::Filter::IsIdle);
	sendIdleWorkersToMinerals();
}

void ProductionManager::onUnitDestroy(bw::Unit unit) {
	m_allUnits.erase(unit);
	m_freeUnits.erase(unit);
}
