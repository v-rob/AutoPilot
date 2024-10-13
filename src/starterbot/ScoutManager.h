#pragma once

#include "IntelManager.h"
#include "Tools.h"
#include "UnitManager.h"

class ScoutManager : public EventReceiver {
private:
    UnitManager& m_unitManager;

    bw::Unitset m_scouts;

public:
    ScoutManager(UnitManager& unitManager);

    bool addScout(bw::UnitType type);
    int countScouts(bw::UnitType type);

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onUnitDestroy(bw::Unit unit) override;
};