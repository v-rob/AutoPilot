#pragma once

#include "BuildingManager.h"
#include "CombatManager.h"
#include "ProductionManager.h"
#include "ScoutManager.h"
#include "Tools.h"
#include "UnitManager.h"

#include <vector>

// Indicates the types of actions that can be taken in the bot's fixed build order.
enum class ActionType {
    BUILD,
    TRAIN,
    MORPH,
    SCOUT,
    ATTACK,
};

// Describes an action that StrategyManager should tell its subordinate classes to do,
// e.g. training a certain number of worker units, or instructing the bot to attack.
struct ActionItem {
    // The type of action to be performed.
    ActionType action;

    // If a certain ActionItem needs to be completed before this one, this field is set to
    // the index of that action. Otherwise, it should be set to -1.
    int depends;

    // The type of the unit needed for the action, e.g. specifying what type of building
    // should be constructed.
    bw::UnitType type = bw::UnitTypes::None;

    // The number of units that are needed for the action, e.g. choosing how many workers
    // should be trained at this moment.
    int count = 0;
};

// This is the class in charge of the overall strategy. It tells the subordinate classes
// that control different aspects of the bot what to do and when according to its internal
// build order and strategy-making decisions.
class StrategyManager : public EventReceiver {
private:
    UnitManager m_unitManager;

    ProductionManager m_productionManager;
    BuildingManager m_buildingManager;
    ScoutManager m_scoutManager;
    CombatManager m_combatManager;

    // The list of actions that define the fixed build order of the strategy.
    std::vector<ActionItem> m_strategy;

    // A list representing whether each stage of the build order has been completed. A
    // stage may become incomplete again if units involved have been destroyed.
    std::vector<bool> m_completion;

public:
    StrategyManager();

protected:
    virtual void notifyMembers(const bw::Event& event) override;

    virtual void onStart() override;
    virtual void onFrame() override;
};