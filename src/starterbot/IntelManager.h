#pragma once

#include "Tools.h"

#include <unordered_map>

// This class keeps track of known intelligence about the enemy's units. Like UnitManager,
// it maintains sets of enemy units and provides functions that match subsets of these
// units according to a predicate. Since unit information is unknown when units are hidden
// by the fog of war, this class is also concerned with keeping track of the last-known
// information about each enemy unit.
class IntelManager : public EventReceiver {
private:
    // The set of all known enemy units, whether visible or not. This set may include
    // units that no longer exist if they were destroyed in the fog of war.
    bw::Unitset m_enemyUnits;
    // The set of all enemy units that are currently visible.
    bw::Unitset m_visibleUnits;

    // Contains the last known position of each unit. If a unit's last known position is
    // in an area that is not currently hidden by the fog of war, then the position is
    // unknown and hence not contained in this map.
    std::unordered_map<bw::Unit, bw::TilePosition> m_lastPositions;

public:
    // Checks whether a specified unit is an enemy unit, namely by checking if the owner
    // of the unit is in the set of enemy players.
    static bool isEnemy(bw::Unit unit);

    // Gets the last known position of the specified unit, or bw::TilePosition::Unknown if
    // we have no idea where the unit is, if it even exists anymore.
    bw::TilePosition getLastPosition(bw::Unit unit);

    // These functions match enemy units from the set of all known enemy units according
    // to a predicate. Note that BWAPI only gives very limited information about hidden
    // units, so many predicates will only reliably match visible units.
    bw::Unit peekUnit(const bw::UnitFilter& pred);
    bw::Unitset peekUnits(const bw::UnitFilter& pred, int count = INT_MAX);
    int peekCount(const bw::UnitFilter& pred);

    // These functions match enemy units from the set of currently visible enemy units.
    bw::Unit findUnit(const bw::UnitFilter& pred);
    bw::Unitset findUnits(const bw::UnitFilter& pred, int count = INT_MAX);
    int findCount(const bw::UnitFilter& pred);

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onUnitHide(bw::Unit unit) override;
    virtual void onUnitDestroy(bw::Unit unit) override;
};