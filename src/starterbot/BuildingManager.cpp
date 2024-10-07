#include "BuildingManager.h"

#include <algorithm>

BuildingManager::BuildingManager(UnitManager& manager) : m_unitManager(manager) {}

void BuildingManager::onStart()
{
	m_reservedBuildings = bw::Unitset();
}

bool BuildingManager::addTrainRequest(bw::UnitType type)
{
	// we'll need to change this if/when we use Archons; only unit that requires two buildings
	bw::UnitType buildingTypeNeeded = type.whatBuilds().first;

	// TODO: first check to see if there's space in an already reserved building

	bw::Unit reservedBuilding = m_unitManager.reserveUnit(buildingTypeNeeded);

	if (!reservedBuilding || !reservedBuilding->train(type))
	{
		return false;
	}

	m_reservedBuildings.insert(reservedBuilding);

	return true;
}

int BuildingManager::countTrainRequests(bw::UnitType type)
{
	int requests = 0;

	for (bw::Unit building : m_reservedBuildings)
	{
		bw::UnitType::list queue = building->getTrainingQueue();
		requests += std::count(queue.begin(), queue.end(), type);
	}

	return requests;
}

// isn't this the same as Tools::CountUnitsOfType?
int BuildingManager::countTrained(bw::UnitType type)
{
	
}

void BuildingManager::onUnitComplete(bw::Unit unit)
{
	// I'm not sure if there's a way to see which building trained a certain unit,
	// so for now I'm just looping through all of them and removing the one with an empty queue

	for (bw::Unit building : m_reservedBuildings)
	{
		if (building->getTrainingQueue().size() == 0)
		{
			m_unitManager.releaseUnit(building);
			m_reservedBuildings.erase(building);
			return;
		}
	}
}