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
    bw::TilePosition pos = g_game->getBuildLocation(
        type, g_self->getStartLocation(), 64, type.requiresCreep());
    if (!pos.isValid()) {
        return false;
    }

    // Reserve a worker to construct or morph into the building, if we have one.
    bw::Unit builder = m_unitManager.reserveUnit(bw::GetType == type.whatBuilds().first);
    if (builder == nullptr) {
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


    // Count how many refineries we have to determine the number of workers needed
    // to collect gas. In this case we need 2 per refinery.
    int gasCount = m_unitManager.borrowCount(bw::Filter::IsRefinery) * 2;

    // Check to see if refinery exists
    if (m_gasGatherers.size() < gasCount) {
        bw::Unitset newGatherers = m_unitManager.reserveUnits(bw::Filter::IsWorker, 2 - m_gasGatherers.size());
        m_gasGatherers.insert(newGatherers.begin(), newGatherers.end());
    }

    // If a unit in already gathering gas, leave it be
    for (bw::Unit unit : m_gasGatherers) {
        if (unit->isGatheringGas()) {
            continue;
        }

        // Get the closest refinery to the starting location
        bw::Unit refinery = g_game->getClosestUnit(
            bw::Position(g_self->getStartLocation()), bw::Filter::IsRefinery);

        // Check if the refinery is null, if not, gather gas
        if (refinery != nullptr) {
            unit->gather(refinery);
        }
    }

    // We borrow as many workers as we can that are not already reserved by some manager
    // and instruct them to gather resources.
    for (bw::Unit unit : m_unitManager.borrowUnits(bw::IsWorker)) {
        // If the unit is already gathering minerals, leave them be.
        if (unit->isGatheringMinerals()) {
            continue;
        }

        // We want to get the mineral that is closest to the base rather than closest to
        // the worker since the worker may be a long distance away from the base.
        bw::Unit mineral = g_game->getClosestUnit(
            bw::Position(g_self->getStartLocation()), bw::IsMineralField);

        // If a suitable mineral was found, instruct the worker to gather it.
        if (mineral != nullptr) {
            unit->gather(mineral);
        }
    }
}

void ProductionManager::onUnitDestroy(bw::Unit unit) {
    m_builders.erase(unit);
    m_trainers.erase(unit);
}