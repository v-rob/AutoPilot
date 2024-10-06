#pragma once

#include "Tools.h"
#include "EventReceiver.h"

class UnitManager : protected EventReceiver {
public:
	bw::Unit borrowUnit(const bw::UnaryFilter<bw::Unit>& pred);
	bw::Unitset borrowUnits(const bw::UnaryFilter<bw::Unit>& pred, int count = -1);
	bw::Unit reserveUnit(const bw::UnaryFilter<bw::Unit>& pred);
	bw::Unitset reserveUnits(const bw::UnaryFilter<bw::Unit>& pred, int count = -1);
	void releaseUnit(bw::Unit unit);
	void releaseUnits(bw::Unitset units);

protected:
	void onStart() override;
	void onFrame() override;
	void onUnitDestroy(bw::Unit unit) override;
	void onUnitComplete(bw::Unit unit) override;

private:
	UnitManager& m_unitManager;
	bw::Unitset m_reservedBuildings;
};