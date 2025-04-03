#include "ProductionManager.h"

ProductionManager::ProductionManager(UnitManager& unitManager) :
    m_unitManager(unitManager) {
}

int ProductionManager::freeMinerals() {
    // Starting with the player's current amount of minerals, subtract the mineral cost of
    // each build request that is pending.
    int minerals = g_self->minerals();

    for (bw::Unit builder : m_builders) {
        // As explained in countBuildRequests(), we only want to include buildings that
        // haven't been placed yet, so we disclude builders with the following conditions.
        if (builder->getBuildUnit() == nullptr && !builder->isMorphing()) {
            minerals -= builder->getBuildType().mineralPrice();
        }
    }

    // In a few edge cases, it's possible to try to place more buildings than we have
    // resources for (meaning the build request will fail when trying to place the
    // building), so make sure this value is nonnegative.
    return std::max(minerals, 0);
}

int ProductionManager::freeGas() {
    // The calculation for the amount of free gas is identical for that for minerals.
    int gas = g_self->gas();

    for (bw::Unit builder : m_builders) {
        if (builder->getBuildUnit() == nullptr && !builder->isMorphing()) {
            gas -= builder->getBuildType().gasPrice();
        }
    }

    return std::max(gas, 0);
}

bool ProductionManager::hasEnoughResources(bw::UnitType type) {
    return type.mineralPrice() <= freeMinerals() && type.gasPrice() <= freeGas();
}

bool ProductionManager::addBuildRequest(bw::UnitType type) {
    // If we don't have enough resources to build a building of this type, then don't
    // waste time by sending a unit to try to place a building that is guaranteed to fail.
    if (!hasEnoughResources(type)) {
        return false;
    }

    // Reserve a worker to construct or morph into the building, if we have one. Since
    // finding a build position is a really slow operation, it is better to fail quickly
    // if we don't have a unit rather than suffer the extra latency.
    bw::Unit builder = m_unitManager.reserveUnit(bw::GetType == type.whatBuilds().first);
    if (builder == nullptr) {
        return false;
    }

    // Get a build location in a reasonable range near the center of the base.
    bw::TilePosition pos;

    if (type.isRefinery()) {
        // If this is a refinery, then we have to find a vespene gas geyser manually since
        // getBuildLocation() finds terrible and/or unexplored locations for refineries.
        bw::Unit geyser = g_game->getClosestUnit(bw::Position(g_self->getStartLocation()),
            bw::GetType == bw::UnitTypes::Resource_Vespene_Geyser);
        pos = geyser != nullptr ? geyser->getTilePosition() : bw::TilePositions::Invalid;
    } else {
        // For everything else, getBuildLocation() is sufficient to get a good position.
        pos = g_game->getBuildLocation(type, g_self->getStartLocation(), 64, type.requiresCreep());
    }

    // If we couldn't find a location, or that location is in an unexplored area (meaning
    // nothing can be placed there), release the unit and return false.
    if (!pos.isValid() || !g_game->isExplored(pos)) {
        m_unitManager.releaseUnit(builder);
        return false;
    }

    // Sometimes, a build request will fail partway through, causing the worker to return
    // to whatever they were doing. This means the worker is never released from the
    // reserved set. So, stop the worker before actually sending the build command,
    // causing the worker to become idle if the request fails, resulting in a release.
    builder->stop();

    // Instruct the worker to construct or morph into the building at the chosen location.
    // If this fails, release the unit and fail.
    if (!builder->build(type, pos)) {
        m_unitManager.releaseUnit(builder);
        return false;
    }

    // The build request succeeded, so add this worker to the set of reserved workers.
    m_builders.insert(builder);
    return true;
}

int ProductionManager::countBuildRequests(bw::UnitType type) {
    // We want to count the number of units who have been told to construct or morph into
    // a building of the specified type, but the building does not exist yet. For Protoss,
    // this includes all workers currently assigned to place a building. For Terrans, this
    // only includes workers that have been told to construct a building, but haven't
    // placed it yet, i.e. they have a build type but no build unit. For Zerg, this means
    // workers instructed to place a building that are not yet morphing into the building.
    //
    // Unfortunately, BWAPI has a limitation where the build type of a worker may not show
    // up during a few frames after the order has been issued. So, if the worker is listed
    // as not having any build type, but isn't idle either, we assume that the worker is
    // constructing or morphing into a building of this type.
    //
    // In rare circumstances, this may mean that we overcount the number of build requests
    // that are active for a few frames, but this is a lesser evil than undercounting the
    // build requests, which may result in multiple workers being told to construct the
    // same building since we didn't know that a worker was already requested to do so.
    //
    // In short, this is a complex and ugly condition, but it gets the job done correctly.
    return UnitManager::matchCount(m_builders,
        (bw::BuildType == type && bw::BuildUnit == nullptr && !bw::IsMorphing) ||
        (bw::BuildType == bw::UnitTypes::None && !bw::IsIdle));
}

bool ProductionManager::addTrainRequest(bw::UnitType type, bool morph) {
    // If we don't have enough resources to train this unit given our build requests,
    // don't train anything because that would take away resources from the build requests
    // and possibly make the building placement fail.
    if (!hasEnoughResources(type)) {
        return false;
    }

    // Try to reserve a unit of the appropriate type that trains or morphs into the
    // requested unit, if we have one.
    bw::Unit trainer = m_unitManager.reserveUnit(bw::GetType == type.whatBuilds().first);
    if (trainer == nullptr) {
        return false;
    }

    // Instruct the unit to train or morph into the new unit. If this fails, such as due
    // to insufficient resources, release the unit and fail.
    bool success = morph ? trainer->morph(type) : trainer->train(type);
    if (!success) {
        m_unitManager.releaseUnit(trainer);
        return false;
    }

    // The train request succeeded, so add this unit to the set of reserved trainers.
    m_trainers.insert(trainer);
    return true;
}

bool ProductionManager::targetBuildRequests(bw::UnitType type, int count) {
    // Our current fulfillment of the quota includes both existing buildings and build
    // requests for buildings that have not been placed yet.
    int progress = m_unitManager.selfCount(bw::GetType == type) + countBuildRequests(type);

    while (progress < count && addBuildRequest(type)) {
        progress++;
    }
    return progress == count;
}

bool ProductionManager::targetTrainRequests(bw::UnitType type, int count, bool morph) {
    // For Protoss and Terran, we can just count the number of units of the given type
    // since partially complete units have the same type as complete ones. For Zerg, we
    // need to check the build type if the unit if morphing since the unit type might be
    // an egg. We don't check the build type for non-morphing units since that would
    // result in double-counting partially trained units for non-Zerg races.
    int progress = m_unitManager.selfCount(
        bw::GetType == type || (bw::BuildType == type && bw::IsMorphing));

    while (progress < count && addTrainRequest(type, morph)) {
        progress++;
    }
    return progress == count;
}

void ProductionManager::idleTrainRequests(bw::UnitType type, bool morph) {
    // All we need to do is count how many of the proper type we can reserve and add train
    // requests for each one of them.
    int idle = m_unitManager.borrowCount(bw::GetType == type.whatBuilds().first);

    for (int i = 0; i < idle; i++) {
        addTrainRequest(type, morph);
    }
}

void ProductionManager::groupTrainRequests(
        const std::vector<std::pair<bw::UnitType, double>> &items, bool morph) {
    // Like idleTrainRequests(), we need to know how many requests we can make first.
    int idle = m_unitManager.borrowCount(bw::GetType == items[0].first.whatBuilds().first);

    for (int i = 0; i < idle; i++) {
        // For each request, we need to choose a unit from the list to train according to
        // its probability. First, get a random number between 0 and 1.
        double choice = (double)rand() / RAND_MAX;
        double prob = 0.0;

        // Then, sum up each probability in the list. At any point in the loop, this gives
        // us the desired probability for selecting this or any of the previous units.
        // If our random number falls in this range at any point, we should select the
        // current unit as the one to train.
        for (const auto &item : items) {
            prob += item.second;

            if (choice <= prob) {
                addTrainRequest(item.first, morph);
                break;
            }
        }
    }
}

void ProductionManager::onStart() {
    m_builders.clear();
    m_trainers.clear();
}

void ProductionManager::onFrame() {
    // Release any reserved builder units that aren't constructing a building or morphing
    // into a building type. We do this by checking for idle units since there are brief
    // periods when a unit is not listed as constructing a building even though it is.
    m_unitManager.releaseUnits(m_builders, bw::IsIdle);

    // Release any reserved trainer units that aren't currently training any units or have
    // finished morphing into another unit type.
    m_unitManager.releaseUnits(m_trainers, !bw::IsTraining && !bw::IsMorphing);

    // We base the number of workers who are assigned to gas collection using a proportion
    // of the total number of workers. We subtract the number of current workers currently
    // gathering gas in order to get the number of workers who need to be assigned.
    int workerCount = m_unitManager.borrowCount(bw::IsWorker);
    int targetGasWorkers = workerCount / 17 -
        m_unitManager.borrowCount(bw::IsWorker && bw::IsGatheringGas);

    // Borrow as many workers as possible that aren't reserved by any manager and send
    // them to collect resources if they aren't already doing so.
    bw::Unitset idleWorkers = m_unitManager.borrowUnits(
        bw::IsWorker && !bw::IsGatheringMinerals && !bw::IsGatheringGas);

    for (bw::Unit unit : idleWorkers) {
        bw::Unit resource = nullptr;

        // If we want gas gatherers, find the nearest refinery and decrement the target
        // number. We want to get the refinery that is closest to the base rather than
        // closest to the worker since the worker may be far away from the base.
        if (targetGasWorkers > 0) {
            resource = g_game->getClosestUnit(
                bw::Position(g_self->getStartLocation()), bw::IsRefinery);
            targetGasWorkers--;
        }

        // If we don't need any more gas gatherers or there are no refineries available,
        // find the nearest mineral instead. The same distance calculation applies.
        if (resource == nullptr) {
            resource = g_game->getClosestUnit(
                bw::Position(g_self->getStartLocation()), bw::IsMineralField);
        }

        // If a suitable resource was found, instruct the worker to gather it.
        if (resource != nullptr) {
            unit->gather(resource);
        }
    }
}

void ProductionManager::onUnitDestroy(bw::Unit unit) {
    m_builders.erase(unit);
    m_trainers.erase(unit);
}