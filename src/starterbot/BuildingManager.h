#pragma once

#include "Tools.h"
#include "UnitManager.h"

class BuildingManager : public EventReceiver {
private:
	UnitManager& m_unitManager;
	bw::Unitset m_reservedBuildings;

public:
	BuildingManager(UnitManager& manager);
	bool addTrainRequest(bw::UnitType type);
	int countTrainRequests(bw::UnitType type);
	int countTrained(bw::UnitType type);

protected:
	void onStart() override;
	void onFrame() override;
	void onUnitDestroy(bw::Unit unit) override;
	void onUnitComplete(bw::Unit unit) override;
};