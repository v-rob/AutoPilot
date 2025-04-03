#pragma once

#include "Tools.h"
#include "StrategyManager.h"

// This class is in charge of handling administrative functions of the bot, such as
// modifying BWAPI configuartion settings and drawing debugging information. All actions
// pertaining to the game itself are managed by StrategyManager.
class AutoPilotBot : public EventReceiver {
private:
	StrategyManager m_strategyManager;

	int m_gameCount = 0;

public:
	// Creates an instance of AutoPilotBot, connects to the BWAPI client, and plays games.
	static void runBot();

protected:
	virtual void notifyMembers(const bw::Event& event) override;

	virtual void onStart() override;
	virtual void onEnd(bool isWinner) override;

private:
	// We don't want people to construct AutoPilotBot except by calling runBot().
	AutoPilotBot() = default;

	// Tries to connect to the BWAPI client repeatedly, waiting if the connection failed.
	// Upon connecting, this calls playGame() repeatedly until the client disconnects.
	void initLoop();
	// Waits for a game to start, and then takes every event from BWAPI as they come and
	// dispatches them to notifyReceiver() until the game stops or the client disconnects.
	void playGame();
};