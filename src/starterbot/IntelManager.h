#pragma once

#include "Tools.h"

#include <unordered_map>

class IntelManager : public EventReceiver {
private:
    bw::Unitset m_enemyUnits;
    bw::Unitset m_visibleUnits;

    std::unordered_map<bw::Unit, bw::TilePosition> m_lastPositions;

public:
    static bool isEnemy(bw::Unit unit);

    bw::TilePosition getLastPosition(bw::Unit unit);

    bw::Unit peekUnit(const bw::UnitFilter& pred);
    bw::Unitset peekUnits(const bw::UnitFilter& pred, int count = INT_MAX);
    int peekCount(const bw::UnitFilter& pred);

    bw::Unit findUnit(const bw::UnitFilter& pred);
    bw::Unitset findUnits(const bw::UnitFilter& pred, int count = INT_MAX);
    int findCount(const bw::UnitFilter& pred);

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onUnitDestroy(bw::Unit unit) override;
};