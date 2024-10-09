#pragma once

#include "Tools.h"
#include "UnitManager.h"

class BuildingManager : public EventReceiver {
private:
    UnitManager& m_unitManager;

    bw::Unitset m_buildings;

public:
    BuildingManager(UnitManager& unitManager);

    bool addTrainRequest(bw::UnitType type);

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onUnitDestroy(bw::Unit unit) override;
};