#pragma once

#include "Tools.h"
#include "UnitManager.h"
#include "IntelManager.h"

class CombatManager : public EventReceiver {
private:
    UnitManager& m_unitManager;
    IntelManager& m_intelManager;

    const int m_targetRadius;
    bw::Unitset m_targetBuildings;
    bw::Unit m_target;

    bool m_attacking;
    bw::Unitset m_army;

public:
    CombatManager(UnitManager& unitManager, IntelManager& intelManager);

    void chooseNewTarget(bw::Unit target);
    void attack();

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
};