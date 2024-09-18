#pragma once

#include <BWAPI.h>
#include <vector>
#include <string>

// implements this: https://liquipedia.net/starcraft/Build_order

class BuildOrder
{

public:

	//// STATIC ////

	// static map of build orders and their names
	static std::map<std::string, BuildOrder> orders;

	// called once, instantiates known build orders
	static void createBuildOrders();


	//// MEMBER ////

	// name of the build order
	const char* m_name;

	// ordered array of supply level / unit type pairs
	std::vector<std::pair<int, BWAPI::UnitType>> m_order;

	// constructor
	BuildOrder(const char* name, const std::vector<std::pair<int, BWAPI::UnitType>> order);
};

