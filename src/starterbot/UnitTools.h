#pragma once

#include "Tools.h"

#include <algorithm>
#include <vector>

// Gets the squared distance between two positions, which avoids the use of a potentially
// expensive square root operation.
int getSquaredDistance(bw::Position a, bw::Position b);

// Checks if a position is within a certain radius of another position.
bool isInRadius(bw::Position a, bw::Position b, int radius);
// Checks if a position is inside of a rectangle bounded by two positions.
bool isInRectangle(bw::Position pos, bw::Position topLeft, bw::Position botRight);

// These functions mirror their BWAPI counterparts in bw::Game, but take an explicit set
// of units rather than using all visible units. Thus, these functions can be used with
// shadow units or reserved units from UnitManager.
bw::Unitset getUnitsInRadius(const bw::Unitset& units, bw::Position pos, int radius);
bool hasUnitInRadius(const bw::Unitset& units, bw::Position pos, int radius);

bw::Unitset getUnitsInRectangle(
    const bw::Unitset& units, bw::Position topLeft, bw::Position botRight);
bool hasUnitInRectangle(const bw::Unitset& units, bw::Position topLeft, bw::Position botRight);

bw::Unit getClosestUnit(const bw::Unitset& units, bw::Position pos);

struct Cluster {
    bw::Unitset units;
    bw::Position centroid;
};

// Takes a set of units and groups them into clusters of units that are close together,
// using a modified non-stochastic version of the k-means++ algorithm that allows
// specifying an optional maximum size for unit clusters.
std::vector<Cluster> findUnitClusters(
    const bw::Unitset& units, int desiredSize, int maxSize = INT_MAX);

// Takes a set of units and groups them into clusters of units that all are within a
// certain radius of each other. This is a simpler clustering algorithm, but works better
// for units that are sparsely and predictably distributed around the map in dense
// clusters such as mineral fields, as opposed to dense and randomly distributed units
// like buildings and units within a players base.
std::vector<Cluster> findRadialClusters(const bw::Unitset& units, int radius);
