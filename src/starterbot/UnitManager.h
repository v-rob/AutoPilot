#pragma once

#include "Tools.h"

#include <climits>

class UnitManager : public EventReceiver {
private:
    bw::Unitset m_allUnits;
    bw::Unitset m_freeUnits;

public:
    static bw::Unit matchUnit(const bw::Unitset& units, const bw::UnitFilter& pred = nullptr);
    static bw::Unitset matchUnits(const bw::Unitset& units,
        const bw::UnitFilter& pred = nullptr, int count = INT_MAX);
    static int matchCount(const bw::Unitset& units, const bw::UnitFilter& pred = nullptr);

    bw::Unit peekUnit(const bw::UnitFilter& pred);
    bw::Unitset peekUnits(const bw::UnitFilter& pred, int count = INT_MAX);
    int peekCount(const bw::UnitFilter& pred);

    bw::Unit borrowUnit(const bw::UnitFilter& pred);
    bw::Unitset borrowUnits(const bw::UnitFilter& pred, int count = INT_MAX);
    int borrowCount(const bw::UnitFilter& pred);

    bw::Unit reserveUnit(const bw::UnitFilter& pred);
    bw::Unitset reserveUnits(const bw::UnitFilter& pred, int count = INT_MAX);

    void releaseUnit(bw::Unit unit);
    void releaseUnits(const bw::Unitset& units);

protected:
    virtual void onStart() override;
    virtual void onUnitComplete(bw::Unit unit) override;
    virtual void onUnitDestroy(bw::Unit unit) override;
};