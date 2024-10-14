#pragma once

#include "BuildingManager.h"
#include "CombatManager.h"
#include "IntelManager.h"
#include "ProductionManager.h"
#include "ScoutManager.h"
#include "Tools.h"
#include "UnitManager.h"

#include <vector>

enum class ActionType {
    BUILD,
    TRAIN,
    SCOUT,
    ATTACK,
};

struct ActionItem {
    static constexpr int NONE = -1;

    ActionType action;
    int depends;

    bw::UnitType type = bw::UnitTypes::None;
    int count = 0;
};

class StrategyManager : public EventReceiver {
private:
    UnitManager m_unitManager;
    IntelManager m_intelManager;

    ProductionManager m_productionManager;
    BuildingManager m_buildingManager;
    ScoutManager m_scoutManager;
    CombatManager m_combatManager;

    std::vector<ActionItem> m_strategy;
    std::vector<bool> m_completion;

public:
    StrategyManager();

protected:
    virtual void notifyMembers(const bw::Event& event) override;

    virtual void onStart() override;
    virtual void onFrame() override;
};