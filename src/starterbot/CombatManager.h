#pragma once

#include "Tools.h"
#include "UnitManager.h"
#include "UnitTools.h"

class CombatManager : public EventReceiver {
private:
    UnitManager& m_unitManager;

    // Indicates whether the bot is currently attacking.
    bool m_attacking;
    int m_retargetTime;

    // The set of all units reserved to be soldiers to attack the enemy.
    bw::Unitset m_soldiers;

public:
    CombatManager(UnitManager& unitManager);

    // Instructs the combat manager to attack. Once called, the bot will attack with full
    // force, and the attack cannot be cancelled.
    void attack();

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onDraw() override;
    virtual void onUnitDestroy(bw::Unit unit) override;

private:
    double pairwise(bw::Unit& attacker, bw::Unit& defender);
    double group(const Cluster& soldiers, const Cluster& targets);
};