#pragma once

#include "Tools.h"
#include "UnitManager.h"

// This class manages the combat of the bot. Currently, it simply sends the bot's full
// force after any enemy units it can find and attacks them. This class will require major
// reworking since it is too simple to be practical.
class CombatManager : public EventReceiver {
private:
    UnitManager& m_unitManager;

    // Indicates whether the bot is currently attacking.
    bool m_attacking;

    // The set of all units reserved to be soldiers to attack the enemy.
    bw::Unitset m_soldiers;

    // This is the enemy unit that the army will attack, or nullptr if we can't find any
    // suitable targets.
    bw::Unit m_target;

public:
    CombatManager(UnitManager& unitManager);

    // Instructs the combat manager to attack. Once called, the bot will attack with full
    // force, and the attack cannot be cancelled.
    void attack();

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onUnitDestroy(bw::Unit unit) override;
};