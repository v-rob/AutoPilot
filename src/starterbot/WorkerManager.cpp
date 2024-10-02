#include "WorkerManager.h"
#include <iostream>
#include <list>

class WorkerManager
{
public:
	std::vector<BWAPI::Unit> getWorkerList(){
		return workerList;
	}
private:
	std::vector<BWAPI::Unit> workerList;
	BWAPI::Unit getWorker();
	void addUnit(const BWAPI::Unit unit);
	void update();
};

void WorkerManager::update() {
	
	return;
};


/*
* @getWorker:
* This method will iterate through the list of all worker units in the 
* class and get a unit that is available for a work assignment. 
* This includes Worker units (UnitType) AND
* Build units (Unit)
*/
BWAPI::Unit WorkerManager::getWorker() {
	// Get all of the workers from self and add it to the list
	std::vector<BWAPI::Unit> currentWorkerList = WorkerManager::getWorkerList();
	const BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
	for (auto unit: BWAPI::Broodwar->self()->getUnits()) {
		if (unit->getType() == workerType) {
			if (unit->isIdle()) {
				return unit;
			}
		}
	}
	//we want to return the worker
};


//Checks if the given unit is in the list,
// if not, add it to the list
void WorkerManager::addUnit(const BWAPI::Unit unit) {
	std::vector<BWAPI::Unit> currentWorkerList = WorkerManager::getWorkerList();
	currentWorkerList.push_back(unit);
};


//void WorkerManager::workerTask(constBWAPI::Unit )