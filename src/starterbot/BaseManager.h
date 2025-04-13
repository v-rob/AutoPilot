#pragma once

#include "Tools.h"
#include "UnitManager.h"
#include <unordered_map>

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

class BaseManager : public EventReceiver {

private:
	const int RESOURCE_RADIUS = 15;

	UnitManager& m_unitManager;

	std::vector<Base> m_bases;
	bw::Unitset m_buildings;

	bool m_drawBases = false;

public:
	BaseManager(UnitManager& unitManager);
	Base& getClosestBase(bw::Unit& unit);

protected:
	virtual void onStart() override;
	virtual void onFrame() override;
	virtual void onDraw() override;
	virtual void onSendText(const std::string& text) override;
};