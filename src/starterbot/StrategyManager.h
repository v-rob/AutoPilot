#pragma once

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

    std::vector<ActionItem> m_strategy;
    int m_strategyItem;

protected:
    virtual void notifyMembers(const bw::Event& event) override;

    virtual void onStart() override;
    virtual void onFrame() override;
};