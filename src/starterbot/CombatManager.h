#pragma once

#include "Tools.h"
#include "UnitManager.h"
#include "IntelManager.h"

// This class manages the combat of the bot. Currently, it simply sends the bot's full
// force after any enemy units it can find and attacks them. This class will require major
// reworking since it is too simple to be practical.
class CombatManager : public EventReceiver {
private:
    UnitManager& m_unitManager;
    IntelManager& m_intelManager;

    // Indicates whether the bot is currently attacking.
    bool m_attacking;

    // The set of all units reserved to be soldiers to attack the enemy.
    bw::Unitset m_soldiers;

    // If suitable enemy units are in sight, this is the one that the army will attack.
    bw::Unit m_target;
    // If no enemy units are in sight, this contains the position that the army will move
    // to in order to find potential units to attack.
    bw::TilePosition m_targetPos;

public:
    CombatManager(UnitManager& unitManager, IntelManager& intelManager);

    // Instructs the combat manager to attack. Once called, the bot will attack with full
    // force, and the attack cannot be cancelled.
    void attack();

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onUnitDestroy(bw::Unit unit) override;
};