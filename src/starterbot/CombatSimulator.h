#pragma once

#include "Tools.h"

#include "ShadowUnit.h"

#include <memory>
#include <vector>

using UnitList = std::vector<bw::Unit>;
using GroupList = std::vector<std::shared_ptr<UnitList>>;

UnitList unitListOf(const bw::Unitset& units);

// Takes a set of units and groups them into clusters of units that are close together,
// using a modified non-stochastic version of the k-means++ algorithm.
GroupList findUnitClusters(int count, const UnitList& units);

// Represents a single attack action, indicating who should attack whom.
struct AttackPairs {
    GroupList self;
    GroupList enemy;
};

AttackPairs runCombatSimulation(UnitList selfUnits, UnitList enemyUnits);