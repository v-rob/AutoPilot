#pragma once

#include "ShadowUnit.h"
#include "Tools.h"

#include <climits>
#include <unordered_map>

// Since BWAPI only does the bare minimum in making units available to the bot, this class
// exists to handle accessing and allocating units.
//
// First, UnitManager stores a ShadowUnit for every known unit in the game. These shadow
// units can be accessed, counted, and queried using methods such as shadowUnits() or
// enemyCount(). The queries for these functions use BWAPI's built-in filters found in the
// bw::Filter namespace. Like normal units, pointers to shadow units will stay valid for
// the entire duration of the game.
//
// Secondly, UnitManager has functionality for keeping track of which units have been
// reserved for use by some manager class. Reserved units may only be used by the manager
// that reserved them until they are released. Only currently non-reserved units may be
// reserved by another manager.
class UnitManager : public EventReceiver {
private:
    // A map from unit IDs to the shadow unit objects maintained by UnitManager. Once a
    // unit is added to this map, it must not be removed until a new game is started in
    // order to ensure that ShadowUnit pointers stay valid.
    std::unordered_map<int, ShadowUnitImpl> m_shadowMap;

    // Sets of all shadow units, the player's shadow units, and the enemy's shadow units.
    // If a unit has been destroyed, it is removed from these sets.
    bw::Unitset m_shadowUnits;
    bw::Unitset m_selfUnits;
    bw::Unitset m_enemyUnits;

    // The set of all completed units that have not been reserved by any manager class.
    bw::Unitset m_freeUnits;

public:
    // Gets the shadow unit corresponding to a normal unit. If (for some reason) no shadow
    // unit exists yet, it will create one. Calling getShadow() on a shadow unit returns
    // the same shadow unit directly.
    ShadowUnit getShadow(bw::Unit unit);

    // Gets the real unit corresponding to a shadow unit. This is generally more
    // convenient than ShadowUnitImpl::getRealUnit(), which may require a cast if a
    // variable has type bw::Unit rather than ShadowUnit. Calling getReal() on a normal
    // unit rather than a shadow unit returns the same unit directly.
    bw::Unit getReal(bw::Unit unit);

    // Queries whether a unit is still alive. This is more specified than the
    // bw::UnitInterface::exists() method, which includes both dead and invisible units.
    // Note that BWAPI doesn't notify the bot when invisible units are destroyed, so this
    // method will still return true for such units.
    bool isAlive(bw::Unit unit);

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

    // These functions query any shadow unit that matches the given predicate.
    bw::Unit shadowUnit(const bw::UnitFilter& pred = nullptr);
    bw::Unitset shadowUnits(const bw::UnitFilter& pred = nullptr, int count = INT_MAX);
    int shadowCount(const bw::UnitFilter& pred = nullptr);

    // These functions query shadow units that are owned by the current (g_self) player.
    bw::Unit selfUnit(const bw::UnitFilter& pred = nullptr);
    bw::Unitset selfUnits(const bw::UnitFilter& pred = nullptr, int count = INT_MAX);
    int selfCount(const bw::UnitFilter& pred = nullptr);

    // These functions query shadow units that are owned by the enemy player.
    bw::Unit enemyUnit(const bw::UnitFilter& pred = nullptr);
    bw::Unitset enemyUnits(const bw::UnitFilter& pred = nullptr, int count = INT_MAX);
    int enemyCount(const bw::UnitFilter& pred = nullptr);

    // These functions match units that are not currently reserved by any manager. This is
    // useful for giving units a temporary command to perform, such as mining minerals.
    // Unlike reserved units, borrowed units may be reserved or borrowed at any time by
    // another manager and given a new task.
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
    // own reserved units and decide if and when to release them.
    void releaseUnit(bw::Unit& unit, const bw::UnitFilter& pred = nullptr);
    void releaseUnits(bw::Unitset& units, const bw::UnitFilter& pred = nullptr);

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onUnitComplete(bw::Unit unit) override;
    virtual void onUnitDestroy(bw::Unit unit) override;
};