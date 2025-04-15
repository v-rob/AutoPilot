#pragma once

#include "CombatManager.h"
#include "ProductionManager.h"
#include "ScoutManager.h"
#include "Tools.h"
#include "UnitManager.h"
#include "BaseManager.h"

// This is the class in charge of the overall strategy. It tells the subordinate classes
// that control different aspects of the bot what to do and when according to its internal
// build order and strategy-making decisions.
class StrategyManager : public EventReceiver {
private:
    UnitManager m_unitManager;

    BaseManager m_baseManager;
    ProductionManager m_productionManager;
    ScoutManager m_scoutManager;
    CombatManager m_combatManager;

public:
    StrategyManager();

protected:
    virtual void notifyMembers(const bw::Event& event) override;

    virtual void onFrame() override;
    virtual void onDraw() override;

private:
    // Since the strategy for each race is substantially different, a different strategy
    // function is called in onFrame() for each race.
    void onProtossFrame();
    void onTerranFrame();
    void onZergFrame();

    // Draws bounding boxes around each unit plus a line indicating what the target of the
    // bot's current command is.
    void drawUnitBoxes();
    void drawCommands();

    // Draws bars above each unit representing the percentage of health, shields, or
    // resources that the unit has.
    void drawHealthBars();
    void drawHealthBar(bw::Unit unit, double ratio, bw::Color color, int yOffset);
};