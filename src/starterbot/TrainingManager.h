#pragma once

#include "Tools.h"
#include "EventReceiver.h"
#include "UnitManager.h"

class TrainingManager : protected EventReceiver {
public:
	TrainingManager(UnitManager& manager);
	bool addTrainRequest(bw::UnitType type);
	int countTrainRequests(bw::UnitType type);
	int countTrained(bw::UnitType type);

protected:
	void onStart() override;
	void onFrame() override;
	void onUnitDestroy(bw::Unit unit) override;
	void onUnitComplete(bw::Unit unit) override;

private:
	UnitManager& m_unitManager;
	bw::Unitset m_reservedBuildings;
};