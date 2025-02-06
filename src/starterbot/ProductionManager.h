#pragma once

#include "Tools.h"
#include "UnitManager.h"

// This class is in charge of handling what each of the bot's workers is doing. It handles
// the gathering of resources and the construction of new buildings.
class ProductionManager : public EventReceiver {
private:
    UnitManager& m_unitManager;

    // The set of workers that are reserved to build a building. Workers that are
    // gathering minerals are not reserved to keep them available for other managers.
    bw::Unitset m_workers;

public:
    ProductionManager(UnitManager& unitManager);

    // Requests that a building of a certain type be constructed. If there are sufficient
    // resources, a place to put the building, and a worker to construct it, then this
    // function returns true. Otherwise, the request fails.
    bool addBuildRequest(bw::UnitType type);

    // Counts the number of pending building requests, meaning that a worker is assigned
    // to construct the building but no building has been placed yet.
    int countBuildRequests(bw::UnitType type);

    // For Zerg units, requests that the unit morphs into another unit. Buildings still
    // require a build command rather than a morph command, even though both appear to be
    // morphing from the user's point of view.
    bool addMorphRequest(bw::UnitType type);

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onUnitDestroy(bw::Unit unit) override;
};