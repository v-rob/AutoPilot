#pragma once

#include <BWAPI.h>
#include <vector>
#include <string>

// implements this: https://liquipedia.net/starcraft/Build_order

class BuildOrder
{
	//// STATIC ////

	// static map of build orders and their names
	static std::map<std::string, BuildOrder> orders;


	//// MEMBER ////

	// name of the build order
	char* m_name;

	// ordered array of supply level / unit type pairs
	std::vector<std::pair<int, BWAPI::UnitType>> m_order;

	// constructor
	BuildOrder(char* name);

	// methods
	void addToOrder(int supply, BWAPI::UnitType type);
};

