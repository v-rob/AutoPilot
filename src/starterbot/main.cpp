#include "AutoPilotBot.h"
#include "Tools.h"

#include <BWAPI/Client.h>

#include <iostream>
#include <thread>
#include <chrono>

bw::Client& g_client = bw::BWAPIClient;

static void playGame(AutoPilotBot& bot) {
    std::cout << "\n### Waiting for game" << std::endl;

    // While there's no game playing, just keep updating the client and looping. If we
    // disconnect at any point, return.
    while (!g_game->isInGame()) {
        if (!g_client.isConnected()) {
            return;
        }

        g_client.update();
    }

    // The game has started, so initialize our global player variable.
    std::cout << "### Game started" << std::endl;
    g_self = g_game->self();

    // Now we have the main bot loop: repeatedly handle any events that come our way and
    // update the client. Again, if we disconnect at any point, return.
    while (g_game->isInGame()) {
        if (!g_client.isConnected()) {
            break;
        }

        for (const bw::Event& event : g_game->getEvents()) {
            bot.notifyReceiver(event);
        }

        g_client.update();
    }

    // Now that the game has finished, increase game count.
    std::cout << "### Game completed" << std::endl;
    g_gameCount++;
}

int main() {
    std::cout << "AutoPilot Bot - University of Portland" << std::endl;
    std::cout << "https://github.com/v-rob/AutoPilot" << std::endl;

    // Try to connect to StarCraft. If it fails, wait a second and try again.
    while (!g_client.connect()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    std::cout << "### Connected" << std::endl;

    // As long as we're connected to StarCraft, keep playing games.
    AutoPilotBot bot;

    while (g_client.isConnected()) {
        playGame(bot);
    }

    std::cout << "\n### Disconnected" << std::endl;
    return 0;
}