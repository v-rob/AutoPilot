#pragma once

#include "Tools.h"

#include <algorithm>
#include <vector>

struct Cluster {
    bw::Unitset units;
    bw::Position centroid;
};

// Calculates the centroid of a set of units by averaging their positions together.
bw::Position findCentroid(const bw::Unitset& units);

// Takes a set of units and groups them into clusters of units that are close together,
// using a modified non-stochastic version of the k-means++ algorithm that allows
// specifying an optional maximum size for unit clusters.
std::vector<Cluster> findUnitClusters(
    const bw::Unitset& units, int desiredSize, int maxSize = INT_MAX);