#include "Tools.h"

bw::GameWrapper& g_game = bw::Broodwar;
bw::Player g_self = nullptr;

int g_gameCount = 0;

void EventReceiver::notifyReceiver(const bw::Event& event) {
    notifyMembers(event);

    switch (event.getType()) {
    case bw::EventType::MatchStart:
        onStart();
        break;
    case bw::EventType::MatchFrame:
        onFrame();
        onDraw();
        break;
    case bw::EventType::MatchEnd:
        onEnd(event.isWinner());
        break;
    case bw::EventType::SendText:
        onSendText(event.getText());
        break;
    case bw::EventType::ReceiveText:
        onReceiveText(event.getPlayer(), event.getText());
        break;
    case bw::EventType::UnitCreate:
        onUnitCreate(event.getUnit());
        break;
    case bw::EventType::UnitDestroy:
        onUnitDestroy(event.getUnit());
        break;
    case bw::EventType::UnitComplete:
        onUnitComplete(event.getUnit());
        break;
    case bw::EventType::UnitMorph:
        onUnitMorph(event.getUnit());
        break;
    case bw::EventType::UnitRenegade:
        onUnitRenegade(event.getUnit());
        break;
    case bw::EventType::UnitHide:
        onUnitHide(event.getUnit());
        break;
    case bw::EventType::UnitShow:
        onUnitShow(event.getUnit());
        break;
    case bw::EventType::UnitEvade:
        onUnitEvade(event.getUnit());
        break;
    case bw::EventType::UnitDiscover:
        onUnitDiscover(event.getUnit());
        break;
    }
}