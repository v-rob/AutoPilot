#include "AutoPilotBot.h"

#include <BWAPI/Client.h>

#include <iostream>
#include <thread>
#include <chrono>

// Chooses the frame time in milliseconds that the game should be run at.
constexpr int LOCAL_SPEED = 10;

bw::Client& g_client = bw::BWAPIClient;

void AutoPilotBot::runBot() {
    AutoPilotBot bot;
    bot.initLoop();
}

void AutoPilotBot::notifyMembers(const bw::Event& event) {
    m_strategyManager.notifyReceiver(event);
}

void AutoPilotBot::onStart() {
    std::cout << "Playing game " << m_gameCount << " on map " << g_game->mapFileName() << std::endl;

    // Set the speed at which our configuration option says the game should be run at.
    g_game->setLocalSpeed(LOCAL_SPEED);
    g_game->setFrameSkip(1);

    // We want the user to be able to send explicit user input.
    g_game->enableFlag(bw::Flag::UserInput);

    // Enable complete map information for transparancy during development.
    g_game->enableFlag(bw::Flag::CompleteMapInformation);
}

void AutoPilotBot::onEnd(bool isWinner) {
    std::cout << "Game finished with " << (isWinner ? "win" : "loss") << std::endl;
}

void AutoPilotBot::initLoop() {
    std::cout << "AutoPilot Bot - University of Portland" << std::endl;
    std::cout << "https://github.com/v-rob/AutoPilot" << std::endl;

    // Try to connect to StarCraft. If it fails, wait a second and try again.
    while (!g_client.connect()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    std::cout << "### Connected" << std::endl;

    // As long as we're connected to StarCraft, keep playing games.
    while (g_client.isConnected()) {
        playGame();
    }

    std::cout << "\n### Disconnected" << std::endl;
}

void AutoPilotBot::playGame() {
    std::cout << "\n### Waiting for game" << std::endl;

    // While there's no game playing, just keep updating the client and looping.
    while (g_client.isConnected() && !g_game->isInGame()) {
        g_client.update();
    }

    // If we disconnect at any point, return and don't try to play a game.
    if (!g_client.isConnected()) {
        return;
    }

    // The game has started, so initialize our global player variable.
    std::cout << "### Game started" << std::endl;
    g_self = g_game->self();

    // Now we have the main bot loop: repeatedly handle any events that come our way and
    // update the client. Again, if we disconnect at any point, return.
    while (g_client.isConnected() && g_game->isInGame()) {
        for (const bw::Event& event : g_game->getEvents()) {
            notifyReceiver(event);
        }

        g_client.update();
    }

    // Now that the game has finished, increase game count.
    std::cout << "### Game completed" << std::endl;
    m_gameCount++;
}

int main() {
    AutoPilotBot::runBot();
    return 0;
}