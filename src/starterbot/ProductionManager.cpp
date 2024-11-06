#include "ProductionManager.h"

ProductionManager::ProductionManager(UnitManager& unitManager) :
    m_unitManager(unitManager) {
}

bool ProductionManager::addBuildRequest(bw::UnitType type) {
    // The game is fairly slow at finding potential build locations, so to prevent
    // calculating it more than necessary, we do a quick mineral check to see if we can
    // even possibly construct this building in the first place.
    if (type.mineralPrice() > g_self->minerals()) {
        return false;
    }

    // Get a build location in a reasonable range near the center of the base. If no
    // location could be found, return false.
    bw::TilePosition pos = g_game->getBuildLocation(type, g_self->getStartLocation(), 32, false);
    if (!pos.isValid()) {
        return false;
    }

    // Reserve a worker to construct the building, if we have one.
    bw::Unit worker = m_unitManager.reserveUnit(bw::Filter::GetType == type.whatBuilds().first);
    if (worker == nullptr) {
        return false;
    }

    // Instruct the worker to construct the building at the chosen location. If this
    // fails, release the unit and return false.
    if (!worker->build(type, pos)) {
        m_unitManager.releaseUnit(worker);
        return false;
    }

    // The build request succeeded, so add this worker to the set of reserved workers.
    m_workers.insert(worker);
    return true;
}

int ProductionManager::countBuildRequests(bw::UnitType type) {
    // We want to count the number of units who have been instructed to construct a
    // building of the specified type. However, BWAPI has a limitation where the action of
    // a worker may not show up until a few frames after the order has been issued. So, if
    // the worker is listed as not constructing anything, but isn't idle either, we assume
    // that the worker is constructing a building of this type.
    //
    // In rare circumstances, this may mean that we overcount the number of build requests
    // that are active for a few frames, but this is a lesser evil than undercounting the
    // build requests, which may result in multiple workers being told to construct the
    // same building since we didn't know that a worker was already requested to do so.
    return UnitManager::matchCount(m_workers, bw::Filter::BuildType == type ||
        (!bw::Filter::IsConstructing && !bw::Filter::IsIdle));
}

void ProductionManager::onStart() {
    m_workers.clear();
}

void ProductionManager::onFrame() {
    // Release any workers that are idle, i.e. not constructing any buildings.
    m_unitManager.releaseUnits(m_workers, bw::Filter::IsIdle);

    // We borrow as many workers as we can that are not already reserved by some manager
    // and instruct them to gather resources.
    for (bw::Unit unit : m_unitManager.borrowUnits(bw::Filter::IsWorker)) {
        // If the unit is already gathering minerals, leave them be.
        if (unit->isGatheringMinerals()) {
            continue;
        }

        // We want to get the mineral that is closest to the base rather than closest to
        // the worker since the worker may have be a long distance away from the base.
        bw::Unit mineral = g_game->getClosestUnit(
            bw::Position(g_self->getStartLocation()), bw::Filter::IsMineralField);

        // If a suitable mineral was found, instruct the worker to gather it.
        if (mineral != nullptr) {
            unit->gather(mineral);
        }
    }
}

void ProductionManager::onUnitDestroy(bw::Unit unit) {
    m_workers.erase(unit);
}