#pragma once

#include "Tools.h"
#include "UnitManager.h"
#include "BaseManager.h"
#include "VectorField.h"

// This class is in charge of reserving scouts and sending them to find the enemy base and
// keep tabs on enemy operations by patrolling the area. This manager is simply in charge
// of moving the scouts around, but the information is gathered by UnitManager.
class ScoutManager : public EventReceiver {
private:
    UnitManager& m_unitManager;
    BaseManager m_baseManager;
    VectorField m_vectorField;

    // The set of units that are reserved as scouts.
    bw::Unitset m_scouts;
    bool finishSearchEnemyBase;
    bool maneuverPathAdded;
    std::list<bw::TilePosition> maneuverPath;
    bw::TilePosition enemyBasePos;
    bool reachedPointOne;
    bool reachedPointTwo;
    bool reachedPointMiddle;
    bool goingRight;
    std::deque<bw::TilePosition> startingLocations;

public:
    ScoutManager(UnitManager& unitManager);

    // Requests that a new scout be reserved. If there are no units of the appropriate
    // type left, then this returns false and no scout is reserved.
    bool addScout(bw::UnitType type);

    // Counts the number of units of a certain type that are reserved as scouts.
    int countScouts(bw::UnitType type);

protected:
    virtual void notifyMembers(const bw::Event& event) override;

    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onUnitDestroy(bw::Unit unit) override;
};