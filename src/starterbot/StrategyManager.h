#pragma once

#include "BuildingManager.h"
#include "ProductionManager.h"
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
    ActionType type;
    bw::UnitType unit;
    int count;
};

class StrategyManager : public EventReceiver {
private:
    UnitManager m_unitManager;

    ProductionManager m_productionManager;
    BuildingManager m_buildingManager;

    std::vector<ActionItem> m_strategy;
    int m_strategyItem;

public:
    StrategyManager();

protected:
    virtual void notifyMembers(const bw::Event& event) override;

    virtual void onStart() override;
    virtual void onFrame() override;
};