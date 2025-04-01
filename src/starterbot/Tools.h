#pragma once

#include <BWAPI.h>

#include <string>

// We don't want to have to type obscenely long names like BWAPI::Broodwar all the time,
// so we make some convenience using's and global variables.
namespace bw {
    using namespace BWAPI;
    using namespace BWAPI::Filter;
}

extern bw::GameWrapper& g_game;
extern bw::Player g_self;

// BWAPI does not have a filter for the UnitInterface::getBuildUnit() method. This method
// is useful for the bot, so we implement our own polyfill for it.
namespace BWAPI::Filter {
    extern const CompareFilter<Unit, Unit, Unit(*)(Unit)> BuildUnit;
}

// This is the base class for all classes that receive events from BWAPI. It contains
// virtual methods for each event that is relevant for the bot to respond to, plus a way
// to dispatch events to subordinate classes that also need to receive events.
class EventReceiver {
public:
    virtual ~EventReceiver() = default;

	// Notifies this event receiver when a new event has been received. This first passes
	// the event to subordinate classes via notifyMembers(), and then calls the
	// appropriate virtual event handler on this class.
    void notifyReceiver(const bw::Event& event);

protected:
	// If this event receiver has subordinate classes that need to be notified about new
	// events, this method can be overridden to call the notifyReceiver() method of each
	// subordinate class.
    virtual void notifyMembers(const bw::Event& event) {}

	// Called whenever a game starts with the number of the current game. Classes that
	// derive from EventReceiver are created once and reused for every match, so
	// initialization for each game should happen here rather than in the constructor.
	virtual void onStart() {}

	// Called for every frame of the game. Much of the main bot logic occurs here.
	virtual void onFrame() {}

	// Like onFrame(), this is called for every frame of the game. However, drawing code
	// should be placed here rather than in onFrame().
	virtual void onDraw() {}

	// Called whenever the game ends, plus information about whether the bot won or lost.
	virtual void onEnd(bool isWinner) {}

	// Called whenever the user sends or receives a chat message. This can be used to
	// catch control messages to control bot functionality or enable debugging features.
	virtual void onSendText(const std::string& text) {}
	virtual void onReceiveText(bw::Player player, const std::string& text) {}

	// Called when a new unit is created. Both Zerg morphing and construction of
	// structures over a Vespene geyser count as morphing rather than creation.
	virtual void onUnitCreate(bw::Unit unit) {}

	// Called when a unit is destroyed or removed from the game for some reason.
	virtual void onUnitDestroy(bw::Unit unit) {}

	// Called when a unit changes from incomplete to complete, such as a building that
	// finished construction or a unit that completed training.
	virtual void onUnitComplete(bw::Unit unit) {}

	// Called when a unit changes from one unit type to a different unit type.
	virtual void onUnitMorph(bw::Unit unit) {}

	// Called when a unit changes ownership, such as from a Protoss mind control ability.
	virtual void onUnitRenegade(bw::Unit unit) {}

	// Called when a unit becomes visible or invisible, such as by the fog of war or by
	// using a cloaking ability.
	virtual void onUnitShow(bw::Unit unit) {}
	virtual void onUnitHide(bw::Unit unit) {}

	// Called when a unit becomes accessible or inaccessible. This appears to be the same
	// as onUnitShow() and onUnitHide(), except that they will not be fired when complete
	// map information is enabled for BWAPI.
	virtual void onUnitEvade(bw::Unit unit) {}
	virtual void onUnitDiscover(bw::Unit unit) {}
};