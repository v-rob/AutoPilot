//The actual ProductionManager.cpp
#include "ProductionManager.h"
#include "UnitManager.h"

///////////////////////////////////////////////

//Constructor
/*
`ProductionManager` has a list of workers who are reserved to build buildings.
 @m_reservedWorkers - list of workers that are currently idle
*/
ProductionManager::ProductionManager(unitManager& manager): unitManager(manager) {
	m_reservedWorkers();
}

///////////////////////////////////////////////


//addBuildRequest
/*
When the `addBuildRequest()` function is called, the manager will attempt to
reserve a new worker and tell the worker to build the appropriate building. If
no worker could be reserved, the function returns false.
*/
bool addBuildRequests(bw::UnitType buildingType) {
	//TODO:meaning of this
}


//countBuildRequests
/*
The `countBuildRequests()` function counts how many buildings of the specified type
are being built by workers. (in progress builds)
*/
int countBuildRequests(bw::UnitType buildingType) {
	//TODO:meaning of this

}


//countBuildings
/*
The `countBuildings()` function counts how many
buildings of the specified type have been completed.
*/
int countBuildings(bw::UnitType buildingType) {
	int numOfBuildings = Tools::CountUnitsOfType(buildingType, g_self->getUnits());
	return numOfBuildings;
}


////////////////////////////////////////////////


void ProductionManager::onStart() {
	m_allUnits.clear();
	m_freeUnits.clear();
}

void ProductionManager::onUnitDestroy(bw::Unit unit) {
	m_allUnits.erase(unit);
	m_freeUnits.erase(unit);
}

void ProductionManager::onUnitComplete(bw::Unit unit) {
	m_allUnits.erase(unit);
	m_freeUnits.erase(unit)
}