#include "StrategyManager.h"

StrategyManager::StrategyManager() :
    m_productionManager(m_unitManager),
    m_scoutManager(m_unitManager),
    m_combatManager(m_unitManager) {
}

void StrategyManager::notifyMembers(const bw::Event& event) {
    m_unitManager.notifyReceiver(event);

    m_productionManager.notifyReceiver(event);
    m_scoutManager.notifyReceiver(event);
    m_combatManager.notifyReceiver(event);
}

void StrategyManager::onFrame() {
    if (g_self->getRace() == bw::Races::Protoss) {
        onProtossFrame();
    } else if (g_self->getRace() == bw::Races::Terran) {
        onTerranFrame();
    } else {
        onZergFrame();
    }
}

void StrategyManager::onProtossFrame() {
    // First, we make sure none of our buildings are idle by training new units.
    m_productionManager.idleTrainRequests(bw::UnitTypes::Protoss_Probe, false);
    m_productionManager.idleTrainRequests(bw::UnitTypes::Protoss_Zealot, false);

    // Make sure we have enough pylons to support the number of units we have. Since
    // Protoss units are relatively slow to make, we don't need to keep as much slack in
    // our supply amount as the other races.
    m_productionManager.targetBuildRequests(
        bw::UnitTypes::Protoss_Pylon, g_self->supplyUsed() / 14);

    int workerCount = m_unitManager.selfCount(bw::GetType == bw::UnitTypes::Protoss_Probe);
    int fighterCount = m_unitManager.selfCount(bw::GetType == bw::UnitTypes::Protoss_Zealot);

    // If we have enough workers with which to gather resources, we can send one of them
    // out to go do some scouting.
    if (workerCount >= 8 && m_scoutManager.countScouts(bw::UnitTypes::Protoss_Probe) == 0) {
        m_scoutManager.addScout(bw::UnitTypes::Protoss_Probe);
    }

    // If we have a sufficient number of workers, we can start expanding our army. We
    // choose the number of gateways to build based on the number of fighting units we
    // currently have so we don't run into bottlenecks in army production.
    if (workerCount >= 11) {
        m_productionManager.targetBuildRequests(
            bw::UnitTypes::Protoss_Gateway, fighterCount / 5 + 1);
    }
}

void StrategyManager::onTerranFrame() {
    // Terran strategy is almost identical to Protoss strategy, but given the faster
    // building speed, some of the constants have been tweaked for somewhat more optimal
    // resource usage.
    m_productionManager.idleTrainRequests(bw::UnitTypes::Terran_SCV, false);
    m_productionManager.idleTrainRequests(bw::UnitTypes::Terran_Marine, false);

    m_productionManager.targetBuildRequests(
        bw::UnitTypes::Terran_Supply_Depot, g_self->supplyUsed() / 12);

    int workerCount = m_unitManager.selfCount(bw::GetType == bw::UnitTypes::Terran_SCV);
    int fighterCount = m_unitManager.selfCount(bw::GetType == bw::UnitTypes::Terran_Marine);

    if (workerCount >= 7 && m_scoutManager.countScouts(bw::UnitTypes::Terran_SCV) == 0) {
        m_scoutManager.addScout(bw::UnitTypes::Terran_SCV);
    }

    if (workerCount >= 10) {
        m_productionManager.targetBuildRequests(
            bw::UnitTypes::Terran_Barracks, fighterCount / 6 + 1);
    }
}

void StrategyManager::onZergFrame() {
    // Before we morph all the larva we have this frame, check to see if we need an
    // overlord. We always want a nice slack in our supply amount, especially because of
    // the fast morph time of Zerg units.
    m_productionManager.targetTrainRequests(
        bw::UnitTypes::Zerg_Overlord, g_self->supplyUsed() / 12 + 1, true);

    // The rest of our larva can be spent on morphing new workers and fighters.
    m_productionManager.groupTrainRequests({
        {bw::UnitTypes::Zerg_Drone,    0.40},
        {bw::UnitTypes::Zerg_Zergling, 0.60},
    }, true);

    int workerCount = m_unitManager.selfCount(bw::GetType == bw::UnitTypes::Zerg_Drone);
    int fighterCount = m_unitManager.selfCount(bw::GetType == bw::UnitTypes::Zerg_Broodling);

    // If we have enough workers past a certain threshold, we can make a spawning pool in
    // order to gain the ability to morph fighter units.
    if (workerCount >= 11) {
        m_productionManager.targetBuildRequests(bw::UnitTypes::Zerg_Spawning_Pool, 1);
    }

    // We want to make sure our production of new units always stays fast, so we build
    // extra hatcheries whenever we exceed a certain number of units.
    m_productionManager.targetBuildRequests(
        bw::UnitTypes::Zerg_Hatchery, (workerCount + fighterCount) / 15 + 1);

    // Like the other races, send a scout once we have a decent number of workers.
    if (workerCount >= 9 && m_scoutManager.countScouts(bw::UnitTypes::Zerg_Drone) == 0) {
        m_scoutManager.addScout(bw::UnitTypes::Zerg_Drone);
    }
}
