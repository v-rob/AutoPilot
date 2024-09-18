#include "BuildOrder.h"

//// STATIC ////

std::map<std::string, BuildOrder> BuildOrder::orders;

void BuildOrder::createBuildOrders()
{
    // Some Protoss vs Protoss build orders

    BuildOrder("Dragoon First", {
        { 8,  BWAPI::UnitTypes::Enum::Protoss_Pylon },
        { 10, BWAPI::UnitTypes::Enum::Protoss_Gateway },
        { 12, BWAPI::UnitTypes::Enum::Protoss_Assimilator },
        { 13, BWAPI::UnitTypes::Enum::Protoss_Cybernetics_Core },
        { 15, BWAPI::UnitTypes::Enum::Protoss_Pylon },
        { 17, BWAPI::UnitTypes::Enum::Protoss_Dragoon }
    });


    BuildOrder("9/9 Horror Gateways", {
        { 6,  BWAPI::UnitTypes::Enum::Protoss_Pylon },
        { 8,  BWAPI::UnitTypes::Enum::Protoss_Gateway },
        { 9,  BWAPI::UnitTypes::Enum::Protoss_Gateway },
        { 11, BWAPI::UnitTypes::Enum::Protoss_Zealot },
        { 13, BWAPI::UnitTypes::Enum::Protoss_Pylon },
        { 13, BWAPI::UnitTypes::Enum::Protoss_Zealot },
        { 15, BWAPI::UnitTypes::Enum::Protoss_Zealot },
    });
}


//// MEMBER ////

// constructor for BuildOrder
BuildOrder::BuildOrder(const char* name, const std::vector<std::pair<int, BWAPI::UnitType>> order)
: m_name(name), m_order(order)
{
    // add new build order to the static map
	orders.insert({ m_name, *this });
}