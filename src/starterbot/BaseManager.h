#pragma once

#include "Tools.h"
#include "UnitManager.h"
#include <unordered_map>

// A Base struct is created for every resource cluster on the map. Each base holds references
// to the resources and buildings placed within it, and information on whether the enemy is
// currently occupying it.
struct Base {

	// The weighted center of the resources in this base.
	bw::Position centroid;

	// The minerals and geysers that belong to this base.
	std::vector<bw::Unit> minerals;
	std::vector<bw::Unit> geysers;

	// All of the buildings closest to this base.
	std::vector<bw::Unit> buildings;
	
	int enemyCount = 0;

	// Returns whether there are enemy buildings in this base.
	bool isEnemyOccupied() const {
		return enemyCount != 0;
	}
};


// This class is in charge of performing basic map analysis to locate potential expansion
// points with resources, or bases, that either player may advance towards. Because BWAPI 
// does not provide these locations (other than the starting positions with g_game->getStartLocations()),
// we find them by clustering minerals and geysers.
class BaseManager : public EventReceiver {

private:
	UnitManager& m_unitManager;

	// The assumed maximum distance between resources in the same cluster, in TilePosition units.
	const int RESOURCE_RADIUS = 15;

	// All Bases on the map.
	std::vector<Base> m_bases;

	// The set of all alive buildings on the map (ours and the enemies).
	bw::Unitset m_buildings;

	bool m_drawBases = false;

public:
	BaseManager(UnitManager& unitManager);

	// Returns the closest base to a given unit based on BroodWar's built in pathfinding, i.e. with 
	// respect to the terrain and not just straight-distance measurement.
	Base& getClosestBase(bw::Unit& unit);

protected:
	virtual void onStart() override;
	virtual void onFrame() override;
	virtual void onDraw() override;
	virtual void onSendText(const std::string& text) override;
};