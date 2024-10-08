#pragam once

#include "Tools.h"

//The actual ProductionManager.h

//good use of #include <climits> e.g. INT_MAX

class ProductionManager : public EventReceiver {
private:
	bw::Unitset m_allUnits;
	bw::Unitset m_freeUnits;
	UnitManager& unitManager; //store a reference to UnitManager(?)
	bw::Unitset m_reservedWorkers; //a set of reserved workers


public:
	//what are the matchUnit/matchUnits/matchCount coming from?
	//what are peekUnit/peekUnits/peekCount coming from?


	//ProductionManager(UnitManager& manager)
	ProductionManager(UnitManager& manager);

	bool addBuildRequest(bw::UnitType);
	int countBuildRequests(bw::UnitType);
	int countBuildings(bw::UnitType);


protected:
	virtual void onStart() override;
	virtual void onFrame() override;
	virtual void onUnitDestroy(bw::Unit unit) override;

};