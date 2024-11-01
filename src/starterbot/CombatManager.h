#pragma once

#include "Tools.h"
#include "UnitManager.h"
#include "IntelManager.h"

class CombatManager : public EventReceiver {
private:
    UnitManager& m_unitManager;
    IntelManager& m_intelManager;

    bool m_attacking;
    bw::Unitset m_soldiers;

    bw::Unit m_target;
    bw::TilePosition m_targetPos;

public:
    CombatManager(UnitManager& unitManager, IntelManager& intelManager);

    void attack();

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onUnitDestroy(bw::Unit unit) override;
};