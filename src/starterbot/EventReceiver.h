#pragma once

#include "Tools.h"

class EventReceiver {
public:
	virtual void notifyReceiver(const bw::Event& event);

protected:
	virtual void notifyMembers(const bw::Event& event);
	virtual void onStart();
	virtual void onFrame();
	virtual void onDraw();
	virtual void onEnd(bool isWinner);
	virtual void onSendText(const std::string& text);
	virtual void onReceiveText(bw::Player player, const std::string& text);
	virtual void onUnitCreate(bw::Unit unit);
	virtual void onUnitDestroy(bw::Unit unit);
	virtual void onUnitComplete(bw::Unit unit);
	virtual void onUnitMorph(bw::Unit unit);
	virtual void onUnitRenegade(bw::Unit unit);
	virtual void onUnitDiscover(bw::Unit unit);
	virtual void onUnitShow(bw::Unit unit);
	virtual void onUnitHide(bw::Unit unit);
};