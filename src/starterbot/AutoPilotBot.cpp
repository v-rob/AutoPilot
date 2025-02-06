#include "AutoPilotBot.h"

// Chooses the frame time in milliseconds that the game should be run at.
constexpr int LOCAL_SPEED = 10;

void AutoPilotBot::notifyMembers(const bw::Event& event) {
    m_strategyManager.notifyReceiver(event);
    m_vectorField.notifyReceiver(event);
}

void AutoPilotBot::onStart() {
    std::cout << "Playing game " << g_gameCount << " on map " << g_game->mapFileName() << std::endl;

    // Set the speed at which our configuration option says the game should be run at.
    g_game->setLocalSpeed(LOCAL_SPEED);
    g_game->setFrameSkip(1);

    // We want the user to be able to send explicit user input.
    g_game->enableFlag(bw::Flag::UserInput);

    m_vectorField.onStart();
}

void AutoPilotBot::onDraw() {
    // Draw some useful information about each unit, namely commands and health.
    drawUnitBoxes();
    drawCommands();
    drawHealthBars();
}

void AutoPilotBot::onEnd(bool isWinner) {
    std::cout << "Game finished with " << (isWinner ? "win" : "loss") << std::endl;
}

void AutoPilotBot::drawUnitBoxes() {
    for (bw::Unit unit : g_game->getAllUnits()) {
        bw::Position topLeft(unit->getLeft(), unit->getTop());
        bw::Position bottomRight(unit->getRight(), unit->getBottom());

        g_game->drawBoxMap(topLeft, bottomRight, bw::Colors::White);
    }
}

void AutoPilotBot::drawCommands() {
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

void AutoPilotBot::drawHealthBars() {
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

void AutoPilotBot::drawHealthBar(bw::Unit unit, double ratio, bw::Color color, int yOffset) {
    bw::Position pos = unit->getPosition();
    int unitTop = pos.y - unit->getType().dimensionUp();

    // Calculate the dimensions for the bar background and length.
    int left   = pos.x - unit->getType().dimensionLeft();
    int right  = pos.x + unit->getType().dimensionRight();
    int top    = unitTop + yOffset - 10;
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