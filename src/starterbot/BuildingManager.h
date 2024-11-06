#pragma once

#include "Tools.h"
#include "UnitManager.h"

// This class is in charge of handling what each of the bot's buildings is currently
// doing. In particular, it handles the training of new units. However, constructing new
// buildings is done in ProductionManager since that is an action performed by workers.
class BuildingManager : public EventReceiver {
private:
    UnitManager& m_unitManager;

    // The set of all buildings that are currently reserved to perform an activity.
    bw::Unitset m_buildings;

public:
    BuildingManager(UnitManager& unitManager);

    // Requests that a certain unit type be trained. Returns true if a building of the
    // appropriate type was available and sufficient resources are available. Otherwise,
    // the request fails.
    bool addTrainRequest(bw::UnitType type);

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onUnitDestroy(bw::Unit unit) override;
};