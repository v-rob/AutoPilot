#pragma once

#include "Tools.h"

#include "ShadowUnit.h"

#include <vector>

using GroupList = std::vector<bw::Unitset>;

// Takes a set of units and groups them into clusters of units that are close together,
// using a modified non-stochastic version of the k-means++ algorithm.
GroupList findUnitClusters(int count, const bw::Unitset& units);

// Represents a single attack action, indicating who should attack whom.
struct AttackPairs {
    GroupList self;
    GroupList enemy;
};

AttackPairs runCombatSimulation(bw::Unitset selfUnits, bw::Unitset enemyUnits);