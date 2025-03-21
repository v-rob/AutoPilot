#pragma once

#include "Tools.h"
#include "UnitManager.h"

// This class is in charge of handling what each of the bot's workers and buildings are
// doing. It handles the gathering of resources, the training/morphing of units, and the
// construction of new buildings.
class ProductionManager : public EventReceiver {
private:
    UnitManager& m_unitManager;

    // The set of workers that are reserved to build a building or morph into another
    // building. Workers that are gathering minerals are not reserved to keep them
    // available for other managers.
    bw::Unitset m_builders;

    // The set of all buildings that are currently reserved to train some units as well as
    // units that are morphing into a non-building type.
    bw::Unitset m_trainers;

public:
    ProductionManager(UnitManager& unitManager);

    // Requests that a building of a certain type be constructed. If there are sufficient
    // resources, a place to put the building, and a worker to construct it, then this
    // function returns true. Otherwise, the request fails.
    bool addBuildRequest(bw::UnitType type);
    // Counts the number of pending building requests, meaning that a worker is assigned
    // to construct the building but no building has been placed yet.
    int countBuildRequests(bw::UnitType type);

    // Requests that a certain unit type be trained or morphed. Returns true if a building
    // or morph source of the appropriate type was available and sufficient resources are
    // available. Otherwise, the request fails.
    bool addTrainRequest(bw::UnitType type, bool morph);

    // Requests that buildings of a certain type be built up to a certain quota. If the
    // quota is already fulfilled, then this does nothing. Otherwise, it tries to add
    // enough build requests to meet the quota. Returns true if it was able to do so.
    bool targetBuildRequests(bw::UnitType type, int count);
    // Requests that units of a certain type be trained or morphed up to a certain quota,
    // just like targetBuildRequests().
    bool targetTrainRequests(bw::UnitType type, int count, bool morph);

    // Adds train requests to all relevant idle buildings or morphable units to train
    // units of a particular type.
    void idleTrainRequests(bw::UnitType type, bool morph);
    // Adds train requests to all relevant idle buildings or morphable units to train one
    // of many different units. The unit to train is selected according to a probability
    // value between 0 and 1 for each unit. All units in the list must be trainable from
    // the same basic building or morphable unit.
    void groupTrainRequests(const std::vector<std::pair<bw::UnitType, double>>& items, bool morph);

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onUnitDestroy(bw::Unit unit) override;
};