#pragma once

#include "Tools.h"
#include "UnitManager.h"

class ProductionManager : public EventReceiver {
private:
    UnitManager& m_unitManager;

    bw::Unitset m_builders;

public:
    ProductionManager(UnitManager& unitManager);

    bool addBuildRequest(bw::UnitType type);
    int countBuildRequests(bw::UnitType type);

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onUnitDestroy(bw::Unit unit) override;
};