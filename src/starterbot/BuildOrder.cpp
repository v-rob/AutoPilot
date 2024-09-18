#include "BuildOrder.h"

//// STATIC ////

std::map<std::string, BuildOrder> BuildOrder::orders;


//// MEMBER ////

// constructor for BuildOrder
BuildOrder::BuildOrder(char* name) : m_name(name)
{
	orders.insert({ m_name, *this });
}

void BuildOrder::addToOrder(int supply, BWAPI::UnitType type)
{
	m_order.push_back(std::make_pair(supply, type));
}