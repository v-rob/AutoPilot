#pragma once

#include "Tools.h"
#include "UnitManager.h"
#include <unordered_map>

class BaseManager : public EventReceiver {

private:
	const int RESOURCE_RADIUS = 10;

	UnitManager& m_unitManager;

	//std::unordered_map<bw::TilePosition, bw::Unitset> m_baseLocations;
	std::vector<bw::Unitset> m_baseLocations;

public:
	BaseManager(UnitManager& unitManager);

protected:
	virtual void onStart() override;
	virtual void onDraw() override;
};