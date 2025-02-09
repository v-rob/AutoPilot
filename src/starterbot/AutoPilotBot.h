#pragma once

#include "Tools.h"
#include "StrategyManager.h"

// This class is in charge of handling administrative functions of the bot, such as
// modifying BWAPI configuartion settings and drawing debugging information. All actions
// pertaining to the game itself are managed by StrategyManager.
class AutoPilotBot : public EventReceiver {
private:
	StrategyManager m_strategyManager;

protected:
	virtual void notifyMembers(const bw::Event& event) override;

	virtual void onStart() override;
	virtual void onDraw() override;
	virtual void onEnd(bool isWinner) override;

private:
	// Draws bounding boxes around each unit plus a line indicating what the target of the
	// bot's current command is.
	void drawUnitBoxes();
	void drawCommands();

	// Draws bars above each unit representing the percentage of health, shields, or
	// resources that the unit has.
	void drawHealthBars();
	void drawHealthBar(bw::Unit unit, double ratio, bw::Color color, int yOffset);
};