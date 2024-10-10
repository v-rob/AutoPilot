#include "ProductionManager.h"

ProductionManager::ProductionManager(UnitManager& unitManager) :
    m_unitManager(unitManager) {
}

bool ProductionManager::addBuildRequest(bw::UnitType type) {
    bw::TilePosition pos = g_game->getBuildLocation(type, g_self->getStartLocation(), 64, false);
    if (pos == bw::TilePositions::Invalid) {
        return false;
    }

    bw::Unit worker = m_unitManager.reserveUnit(bw::Filter::GetType == type.whatBuilds().first);
    if (worker == nullptr) {
        return false;
    }

    if (!worker->build(type, pos)) {
        m_unitManager.releaseUnit(worker);
        return false;
    }

    m_builders.insert(worker);
    return true;
}

int ProductionManager::countBuildRequests(bw::UnitType type) {
    return UnitManager::matchCount(m_builders, bw::Filter::BuildType == type ||
        (!bw::Filter::IsConstructing && !bw::Filter::IsIdle));
}

void ProductionManager::onStart() {
    m_builders.clear();
}

void ProductionManager::onFrame() {
    m_unitManager.releaseUnits(m_builders, bw::Filter::IsIdle);

    for (bw::Unit unit : m_unitManager.borrowUnits(bw::Filter::IsWorker)) {
        if (unit->isGatheringMinerals()) {
            continue;
        }

        bw::Unit mineral = g_game->getClosestUnit(
            bw::Position(g_self->getStartLocation()), bw::Filter::IsMineralField);

        if (mineral != nullptr) {
            unit->gather(mineral);
        }
    }
}

void ProductionManager::onUnitDestroy(bw::Unit unit) {
    m_builders.erase(unit);
}