#pragma once

#include "Tools.h"
#include "StrategyManager.h"

// Chooses the frame time in milliseconds that the game should be run at.
constexpr int LOCAL_SPEED = 10;

class AutoPilotBot : public EventReceiver {
private:
	StrategyManager m_strategyManager;

protected:
	virtual void notifyMembers(const bw::Event& event) override;

	virtual void onStart() override;
	virtual void onDraw() override;
	virtual void onEnd(bool isWinner) override;

private:
	void drawUnitBoxes();
	void drawCommands();

	void drawHealthBars();
	void drawHealthBar(bw::Unit unit, double ratio, bw::Color color, int yOffset);
};