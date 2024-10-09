//The actual ProductionManager.cpp
#include "ProductionManager.h"

///////////////////////////////////////////////

//Constructor
/*
`ProductionManager` has a list of workers who are reserved to build buildings.
 @m_reservedWorkers - list of workers that are currently idle
*/
ProductionManager::ProductionManager(UnitManager& manager) {
}

///////////////////////////////////////////////


//addBuildRequest
/*
When the `addBuildRequest()` function is called, the manager will attempt to
reserve a new worker and tell the worker to build the appropriate building. If
no worker could be reserved, the function returns false.
*/
bool ProductionManager::addBuildRequest(bw::UnitType buildingType) {
	//TODO:meaning of this
	bw::Unit worker = m_reservedWorkers.reserveUnit(bw::Filter::GetType == buildingType.whatBuilds().first);

		if (worker == nullptr) {
			return false;
		}

		if (!worker->isIdle()) {
			m_reservedWorkers.releaseUnit(worker);
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
	//TODO:meaning of this
	//How many workers are working on one building?
	//uses number of buildings and add

	return UnitManager::matchCount(m_reservedWorkers,bw::Filter::BuildType == buildingType)  
}





void ProductionManager::onStart() {
	m_allUnits.clear();
	m_freeUnits.clear();
}

void ProductionManager::onFrame() {
	m_unitManager.releaseUnits(m_reservedWorkers, !bw::Filter::IsIdle)
}

void ProductionManager::onUnitDestroy(bw::Unit unit) {
	m_allUnits.erase(unit);
	m_freeUnits.erase(unit);
}
