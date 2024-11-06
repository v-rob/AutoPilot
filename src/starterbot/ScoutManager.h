#pragma once

#include "IntelManager.h"
#include "Tools.h"
#include "UnitManager.h"

// This class is in charge of reserving scouts and sending them to find the enemy base and
// keep tabs on enemy operations by patrolling the area. This manager is simply in charge
// of moving the scouts around, but the information is gathered by IntelManager.
class ScoutManager : public EventReceiver {
private:
    UnitManager& m_unitManager;

    // The set of units that are reserved as scouts.
    bw::Unitset m_scouts;

public:
    ScoutManager(UnitManager& unitManager);

    // Requests that a new scout be reserved. If there are no units of the appropriate
    // type left, then this returns false and no scout is reserved.
    bool addScout(bw::UnitType type);

    // Counts the number of units of a certain type that are reserved as scouts.
    int countScouts(bw::UnitType type);

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onUnitDestroy(bw::Unit unit) override;
};