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

void StrategyManager::onDraw() {
    // Draw some useful information about each unit, namely commands and health.
    drawUnitBoxes();
    drawCommands();
    drawHealthBars();
}

void StrategyManager::onProtossFrame() {
    // First, we make sure none of our buildings are idle by training new units.
    m_productionManager.idleTrainRequests(bw::UnitTypes::Protoss_Probe, false);
    m_productionManager.groupTrainRequests({
        {bw::UnitTypes::Protoss_Zealot,  0.60},
        {bw::UnitTypes::Protoss_Dragoon, 0.40},
    }, false);

    // Make sure we have enough pylons to support the number of units we have. Since
    // Protoss units are relatively slow to make, we don't need to keep as much slack in
    // our supply amount as the other races.
    m_productionManager.targetBuildRequests(
        bw::UnitTypes::Protoss_Pylon, g_self->supplyUsed() / 14);

    int workerCount  = m_unitManager.selfCount(bw::GetType == bw::UnitTypes::Protoss_Probe);
    int zealotCount  = m_unitManager.selfCount(bw::GetType == bw::UnitTypes::Protoss_Zealot);
    int dragoonCount = m_unitManager.selfCount(bw::GetType == bw::UnitTypes::Protoss_Dragoon);

    // If we have enough workers with which to gather resources, we can send one of them
    // out to go do some scouting.
    if (workerCount >= 9 && m_scoutManager.countScouts(bw::UnitTypes::Protoss_Probe) == 0) {
        m_scoutManager.addScout(bw::UnitTypes::Protoss_Probe);
    }

    // If we have a sufficient number of workers, we can start expanding our army. We
    // choose the number of gateways to build based on the number of fighting units we
    // currently have so we don't run into bottlenecks in army production.
    if (workerCount >= 11) {
        m_productionManager.targetBuildRequests(
            bw::UnitTypes::Protoss_Gateway, (zealotCount + dragoonCount) / 5 + 1);
    }

    // Once we've built a few zealots, we should start expanding to get more powerful
    // units. So, build an assimilator to start collecting gas, and then a cybernetics
    // core for dragoon support.
    if (zealotCount > 2) {
        m_productionManager.targetBuildRequests(bw::UnitTypes::Protoss_Assimilator, 1);
    }
    if (zealotCount > 3) {
        m_productionManager.targetBuildRequests(bw::UnitTypes::Protoss_Cybernetics_Core, 1);
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

    if (workerCount >= 8 && m_scoutManager.countScouts(bw::UnitTypes::Terran_SCV) == 0) {
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

void StrategyManager::drawUnitBoxes() {
    for (bw::Unit unit : g_game->getAllUnits()) {
        bw::Position topLeft(unit->getLeft(), unit->getTop());
        bw::Position bottomRight(unit->getRight(), unit->getBottom());

        g_game->drawBoxMap(topLeft, bottomRight, bw::Colors::White);
    }
}

void StrategyManager::drawCommands() {
    for (bw::Unit unit : g_self->getUnits()) {
        const bw::UnitCommand& command = unit->getLastCommand();

        // If the previous command had a ground position target, draw it in green.
        if (command.getTargetPosition() != bw::Positions::None) {
            g_game->drawLineMap(unit->getPosition(),
                command.getTargetPosition(), bw::Colors::Green);
        }

        // If the previous command had a unit target, draw it in white.
        if (command.getTarget() != nullptr) {
            g_game->drawLineMap(unit->getPosition(),
                command.getTarget()->getPosition(), bw::Colors::White);
        }
    }
}

void StrategyManager::drawHealthBars() {
    for (bw::Unit unit : g_game->getAllUnits()) {
        // If the unit is a resource, draw the remaining resources in cyan.
        if (unit->getType().isResourceContainer() && unit->getInitialResources() > 0) {
            double mineralRatio = (double)unit->getResources() / (double)unit->getInitialResources();
            drawHealthBar(unit, mineralRatio, bw::Colors::Cyan, 0);
        }

        // If the unit has health, then draw the health in green, orange, or red,
        // corresponding to how damaged the unit is.
        if (unit->getType().maxHitPoints() > 0) {
            double hpRatio = (double)unit->getHitPoints() / (double)unit->getType().maxHitPoints();

            bw::Color hpColor;
            if (hpRatio < 0.33) {
                hpColor = bw::Colors::Red;
            } else if (hpRatio < 0.66) {
                hpColor = bw::Colors::Orange;
            } else {
                hpColor = bw::Colors::Green;
            }

            drawHealthBar(unit, hpRatio, hpColor, 0);
        }

        // If the unit has shields, draw those as well in blue.
        if (unit->getType().maxShields() > 0) {
            double shieldRatio = (double)unit->getShields() / (double)unit->getType().maxShields();
            drawHealthBar(unit, shieldRatio, bw::Colors::Blue, -3);
        }
    }
}

void StrategyManager::drawHealthBar(bw::Unit unit, double ratio, bw::Color color, int yOffset) {
    bw::Position pos = unit->getPosition();
    int unitTop = pos.y - unit->getType().dimensionUp();

    // Calculate the dimensions for the bar background and length.
    int left = pos.x - unit->getType().dimensionLeft();
    int right = pos.x + unit->getType().dimensionRight();
    int top = unitTop + yOffset - 10;
    int bottom = unitTop + yOffset - 6;

    int bar = (int)((right - left) * ratio + left);

    // Draw the background of the bar, the bar itself, and the tick marks inside the bar.
    g_game->drawBoxMap(bw::Position(left, top), bw::Position(right, bottom), bw::Colors::Grey, true);
    g_game->drawBoxMap(bw::Position(left, top), bw::Position(bar, bottom), color, true);
    g_game->drawBoxMap(bw::Position(left, top), bw::Position(right, bottom), bw::Colors::Black, false);

    for (int i = left; i < right - 1; i += 3) {
        g_game->drawLineMap(bw::Position(i, top), bw::Position(i, bottom), bw::Colors::Black);
    }
}