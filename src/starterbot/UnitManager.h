#pragma once

#include "Tools.h"

#include <climits>

// The manager classes in the bot need a way to keep track of which units have been
// reserved for use by some other manager, and which units are free to use. So, each
// manager class gets a reference to UnitManager, which has a set of all the player's
// units plus a set of which units have been reserved. It provides methods for reserving
// and viewing these units.
class UnitManager : public EventReceiver {
private:
    // The set of all units owned by the player that have been completed.
    bw::Unitset m_allUnits;
    // The set of all completed units that have not been reserved by any manager class.
    bw::Unitset m_freeUnits;

public:
    // A static function for matching a single unit out of a set of units according to a
    // predicate. If the predicate is nullptr, any unit will match. If multiple units
    // match the criteria, it is unspecified which unit will be returned. If no units
    // match the critera, nullptr will be returned.
    static bw::Unit matchUnit(const bw::Unitset& units, const bw::UnitFilter& pred = nullptr);

    // Similar to matchUnit(), but returns a set containing all units that match the
    // predicate up to a maximum count of units. The default maximum is unlimited.
    static bw::Unitset matchUnits(const bw::Unitset& units,
        const bw::UnitFilter& pred = nullptr, int count = INT_MAX);

    // A complement to matchUnits() that just counts how many units match the specified
    // predicate without building up a set of units.
    static int matchCount(const bw::Unitset& units, const bw::UnitFilter& pred = nullptr);

    // These functions match units from the set of units owned by this player. If
    // "progress" is true, then it will match units that are in-progress (e.g. being
    // trained or under construction), but if false, it will only match completed units.
    // Units returned from these functions are for informational purposes only: they may
    // be reserved by another manager, so they must not be given any commands.
    bw::Unit peekUnit(const bw::UnitFilter& pred = nullptr, bool progress = false);
    bw::Unitset peekUnits(const bw::UnitFilter& pred = nullptr,
        int count = INT_MAX, bool progress = false);
    int peekCount(const bw::UnitFilter& pred = nullptr, bool progress = false);

    // These functions match units that are not currently reserved by any manager. This is
    // useful for giving units a temporary command to perform, such as mining minerals.
    // These units may be reserved or borrowed at any time by another manager and given a
    // new task.
    bw::Unit borrowUnit(const bw::UnitFilter& pred = nullptr);
    bw::Unitset borrowUnits(const bw::UnitFilter& pred = nullptr, int count = INT_MAX);
    int borrowCount(const bw::UnitFilter& pred = nullptr);

    // These functions reserve units that are not currently reserved by any manager by
    // removing them from the set of free units. The reserving manager is free to give
    // these units any command without fear of another manager messing with them. Reserved
    // units remain reserved until releaseUnits() is called on them. Future calls to
    // borrowUnits() and reserveUnits() will not return any currently reserved units.
    bw::Unit reserveUnit(const bw::UnitFilter& pred = nullptr);
    bw::Unitset reserveUnits(const bw::UnitFilter& pred = nullptr, int count = INT_MAX);

    // Releases currently reserved units. It is up to each manager to keep track of its
    // own reserved units and decide when and if to release them.
    void releaseUnit(bw::Unit& unit, const bw::UnitFilter& pred = nullptr);
    void releaseUnits(bw::Unitset& units, const bw::UnitFilter& pred = nullptr);

protected:
    virtual void onStart() override;
    virtual void onUnitComplete(bw::Unit unit) override;
    virtual void onUnitDestroy(bw::Unit unit) override;
};