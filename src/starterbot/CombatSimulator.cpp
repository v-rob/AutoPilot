#include "CombatSimulator.h"

struct ClusterData {
	std::vector<bw::Position> centroids;
	std::vector<bw::Unitset> clusters;
};

static int getSquaredDistance(bw::Position a, bw::Position b) {
	bw::Position diff = a - b;
	return diff.x * diff.x + diff.y * diff.y;
}

static std::pair<int, int> getClosestCentroid(ClusterData& d, bw::Unit unit, int count) {
	// Initially set the minimum distance to the maximal integer value, which any centroid
	// will match as closer than.
	int minCluster = -1;
	int minDistance = INT_MAX;

	for (int cluster = 0; cluster < count; cluster++) {
		// Get the distance between this unit and the centroid of the cluster we're
		// looking at. If it's smaller than any distance we've found before, select this
		// as the closest cluster so far.
		int distance = getSquaredDistance(d.centroids[cluster], unit->getPosition());

		if (distance < minDistance) {
			minCluster = cluster;
			minDistance = distance;
		}
	}

	return std::make_pair(minCluster, minDistance);
}

static void chooseInitialCentroids(ClusterData& d, const bw::Unitset& units) {
	// We use a somewhat modified version of the k-means++ choice of initial centroids:
	// instead of randomly choosing a position with probability proportional to the
	// squared distance to the nearest centroid, we deterministically choose the position
	// with the largest such distance. This keeps our clusterings stable across frames.
	for (int cluster = 0; cluster < d.clusters.size(); cluster++) {
		// Set the maximal distance to the minimal integer value, which every distance
		// will be greater than.
		bw::Position maxPosition = bw::Positions::Invalid;
		int maxDistance = INT_MIN;

		for (bw::Unit unit : units) {
			// Find the distance from this unit to the nearest centroid. If we don't have
			// any centroids yet, choose the unit that is farthest away from the origin.
			int distance;
			if (cluster == 0) {
				distance = getSquaredDistance(unit->getPosition(), bw::Positions::Origin);
			} else {
				distance = getClosestCentroid(d, unit, cluster).second;
			}

			// If this distance is larger than any one we've seen before, choose it as our
			// best initial guess for a centroid so far.
			if (distance > maxDistance) {
				maxPosition = unit->getPosition();
				maxDistance = distance;
			}
		}

		d.centroids[cluster] = maxPosition;
	}
}

static void repopulateClusters(ClusterData& d, const bw::Unitset& units) {
	// First, we clear out our old clusters so we can regenerate them.
	for (int cluster = 0; cluster < d.clusters.size(); cluster++) {
		d.clusters[cluster].clear();
	}

	// For each unit, put it into the cluster that has the closest centroid.
	for (bw::Unit unit : units) {
		int minCluster = getClosestCentroid(d, unit, (int)d.clusters.size()).first;
		d.clusters[minCluster].insert(unit);
	}
}

static void computeNewCentroids(ClusterData& d) {
	// All the clusters have been populated, so we need to recompute the centroids of
	// these clusters. We do so by averaging the positions of all the points.
	for (int cluster = 0; cluster < d.clusters.size(); cluster++) {
		bw::Position total = bw::Positions::Origin;

		for (bw::Unit unit : d.clusters[cluster]) {
			total += unit->getPosition();
		}

		d.centroids[cluster] = total / (int)d.clusters[cluster].size();
	}
}

std::vector<bw::Unitset> findUnitClusters(int count, const bw::Unitset& units) {
	// We could iterate the k-means++ algorithm until we converge to a set of clusters,
	// but that's really unnecessary for our purposes. Instead, iterate a fixed number of
	// times, which gives us a good enough clustering.
	constexpr int MAX_ITER = 10;

	// It doesn't make sense to have more clusters than units, so clamp the count.
	count = std::min(count, (int)units.size());

	ClusterData d;
	d.centroids.resize(count);
	d.clusters.resize(count);

	// There's no point in doing any computations if we have no clusters at all.
	if (count == 0) {
		return d.clusters;
	}

	// We choose a set of initial centroid for our algorithm to work with. Having a smart
	// initial guess is critical to finding a good clustering.
	chooseInitialCentroids(d, units);

	// Now, we iterate the algorithm for our fixed number of steps, repopulating the
	// cluster sets each iteration and choosing new centroids based on those clusters.
	for (int iter = 0; iter < MAX_ITER; iter++) {
		repopulateClusters(d, units);
		computeNewCentroids(d);
	}

	return d.clusters;
}

// The maximum number of turns we will simulate in a single turn.
static constexpr int MAX_DEPTH = 2;

static void simulateCombatAction(PlayerState& self, PlayerState& enemy, int unit, int target) {
    // For now, we do a random walk by modifying our goodness by a random value in the
    // range [-1, 1]. We don't modify the health of the units or anything.
    self.goodness += ((double)rand() / RAND_MAX) * 2.0 - 1.0;
}

static void simulateCombatTurn(PlayerState& self, PlayerState& enemy,
        int unitIndex, int depth) {
    // If we've reached our maximum possible depth, then we're done simulating this path
    // of the tree.
    if (depth == 0) {
        return;
    }

    // If we've assigned an action for each of our units, run a simulation on the other
    // player for a turn.
    if (unitIndex >= self.units.size()) {
        simulateCombatTurn(enemy, self, 0, depth - 1);
        return;
    }

    // Simulate what happens if we try to attack each possible enemy unit.
    for (int targetIndex = 0; targetIndex < enemy.units.size(); targetIndex++) {
        // Make copies of the states of each player since we're going to mutate the states
        // as a result of our simulation for this unit.
        PlayerState newSelf = self;
        PlayerState newEnemy = enemy;

        // Record which target we attacked with this unit.
        newSelf.attacks.emplace_back(
            newSelf.units[unitIndex].unit, newEnemy.units[targetIndex].unit);

        // Simulate the attack, which will update the goodness value of our player state.
        simulateCombatAction(newSelf, newEnemy, unitIndex, targetIndex);

        // Continue simulating the tree that results from this branch until we max out on
        // the depth.
        simulateCombatTurn(newSelf, newEnemy, unitIndex + 1, depth);

        // Now we can compare the goodness of this branch against the previous best
        // goodness value. If our goodness is better, then set this as the preferred
        // branch of the tree.
        if (newSelf.goodness - newEnemy.goodness > self.goodness - enemy.goodness) {
            self = std::move(newSelf);
            enemy = std::move(newEnemy);
        }
    }
}

AttackList runCombatSimulation(const bw::Unitset& selfUnits, const bw::Unitset& enemyUnits) {
    // Set the state of our players according to the current game state.
    PlayerState self(selfUnits);
    PlayerState enemy(enemyUnits);

    // Recursively simulate the possible game trees, branching whenever we have a choice
    // on who to attack. We multiply the maximum depth by two since we decrement the depth
    // every time we switch players within a single turn.
    simulateCombatTurn(self, enemy, 0, MAX_DEPTH * 2);

    self.attacks.resize(selfUnits.size());
    return self.attacks;
}