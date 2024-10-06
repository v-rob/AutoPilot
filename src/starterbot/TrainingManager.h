#pragma once

#include "Tools.h"
#include "EventReceiver.h"
#include "UnitManager.h"

class TrainingManager : protected EventReceiver {
public:
	TrainingManager(UnitManager& manager);
	bool addTrainRequest(bw::UnitType);
	int countTrainRequests(bw::UnitType);
	int countTrained(bw::UnitType);

protected:
	void onStart() override;
	void onFrame() override;
	void onUnitDestroy(bw::Unit unit) override;

private:
	UnitManager& m_unitManager;
	bw::Unitset m_reservedBuildings;
};