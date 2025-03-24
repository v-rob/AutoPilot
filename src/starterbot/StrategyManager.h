#pragma once

#include "CombatManager.h"
#include "ProductionManager.h"
#include "ScoutManager.h"
#include "Tools.h"
#include "UnitManager.h"

// This is the class in charge of the overall strategy. It tells the subordinate classes
// that control different aspects of the bot what to do and when according to its internal
// build order and strategy-making decisions.
class StrategyManager : public EventReceiver {
private:
    UnitManager m_unitManager;

    ProductionManager m_productionManager;
    ScoutManager m_scoutManager;
    CombatManager m_combatManager;

public:
    StrategyManager();

protected:
    virtual void notifyMembers(const bw::Event& event) override;

    virtual void onFrame() override;

private:
    // Since the strategy for each race is substantially different, a different strategy
    // function is called in onFrame() for each race.
    void onProtossFrame();
    void onTerranFrame();
    void onZergFrame();
};