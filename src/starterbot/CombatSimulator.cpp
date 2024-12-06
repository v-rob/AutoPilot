#include "CombatSimulator.h"

struct ClusterData {
    std::vector<bw::Position> centroids;
    GroupList clusters;
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
        bw::Position maxPosition = bw::Positions::Origin;
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

GroupList findUnitClusters(int count, const bw::Unitset& units) {
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

// The maximum number of turns we will simulate in a single frame.
static constexpr int MAX_TURNS = 20;

// The maximum number of items that we keep in our priority queue.
static constexpr int MAX_QUEUE = 10;

// For now, we split the units owned by each player into a fixed number of clusters for
// our simulation, regardless of whether that will result in an ideal clustering.
static constexpr int MAX_SELF_CLUSTERS = 5;
static constexpr int MAX_ENEMY_CLUSTERS = 8;

struct SimulationState {
    bw::Unitset selfUnits;
    bw::Unitset enemyUnits;

    AttackPairs firstAttacks;

    double goodness = 0.0;
};

static bool compareStates(SimulationState& left, SimulationState& right) {
    // Compare two player states based on their goodness. Strangely, the STL uses a less
    // than comparison to implement a max heap with std::push_heap() and so on, so we have
    // to use a greater than to get a min heap.
    return left.goodness > right.goodness;
}

static void queueNewState(std::vector<SimulationState>& queue, SimulationState state) {
    // We implement the fixed-size priority queue as a min heap. If we've reached the
    // maximum size and the new state is worse than the worst queued state, discard it.
    if (queue.size() >= MAX_QUEUE && state.goodness <= queue.front().goodness) {
        return;
    }

    // Otherwise, this state needs to be queued. If we've reached the maximum size, we
    // need to remove the worst state from the queue.
    if (queue.size() >= MAX_QUEUE) {
        std::pop_heap(queue.begin(), queue.end(), &compareStates);
        queue.pop_back();
    }

    // Then, we add this state into the priority queue.
    queue.push_back(std::move(state));
    std::push_heap(queue.begin(), queue.end(), &compareStates);
}

static SimulationState& findBestState(std::vector<SimulationState>& queue) {
    // Since compareStates() uses a greater than comparison, we use std::min_element() to
    // to find the maximal element in the queue.
    return *std::min_element(queue.begin(), queue.end(), &compareStates);
}

static int computeDefense(const bw::Unitset& units) {
    int defense = 0;
    for (bw::Unit unit : units) {
        defense += unit->getHitPoints() + unit->getShields();
    }
    return defense;
}

static void simulateCombatGroup(SimulationState& state,
        bw::Unitset& selfGroup, bw::Unitset& enemyGroup) {
    // For now, we just have a ridiculously simplistic heuristic function that compares
    // the health and shields of the two groups and modifies the goodness accordingly.
    int selfDefense = computeDefense(selfGroup);
    int enemyDefense = computeDefense(enemyGroup);

    state.goodness += selfDefense - enemyDefense;
}

static void simulateCombatEvent(SimulationState& state, AttackPairs& pairing) {
    // Presumably, aside from the pairwise heuristic function, this function should also
    // modify the heuristics of the game. However, we don't have that as yet.
    for (int i = 0; i < pairing.self.size(); i++) {
        simulateCombatGroup(state, pairing.self[i], pairing.enemy[i]);
    }
}

static void createCombatPairings(std::vector<AttackPairs>& pairings,
        GroupList& selfGroups, GroupList& enemyGroups, int current) {
    // If we've permuted enough of the initial sequence of enemy groups to map the self
    // groups on to them, then push this as one possible pairing and backtrack.
    if (current == selfGroups.size()) {
        pairings.push_back({ selfGroups, enemyGroups });
        return;
    }

    // We need to permute the current index, so swap this group with each possible group
    // after it (including itself, resulting in no swap) and permute the rest of the list.
    // Then reverse the swap and do the next element.
    for (int i = current; i < enemyGroups.size(); i++) {
        enemyGroups[current].swap(enemyGroups[i]);
        createCombatPairings(pairings, selfGroups, enemyGroups, current + 1);
        enemyGroups[current].swap(enemyGroups[i]);
    }
}

static void simulateCombatTurn(std::vector<SimulationState>& queue, SimulationState& initial) {
    // To drastically reduce the search space of possible unit-target combinations, we
    // cluster both player's units by proximity and consider them all together.
    GroupList selfGroups  = findUnitClusters(MAX_SELF_CLUSTERS,  initial.selfUnits);
    GroupList enemyGroups = findUnitClusters(MAX_ENEMY_CLUSTERS, initial.enemyUnits);

    // We create every possible one-to-one pairing that maps each of the player's groups
    // onto a targeted enemy group by finding every possible permutation of the initial
    // sequence of enemy groups.
    std::vector<AttackPairs> pairings;
    createCombatPairings(pairings, selfGroups, enemyGroups, 0);

    // Now we can simulate each of these attack pairings and add them to the priority
    // queue. Any really bad options will fall off the end of the queue, and we'll just be
    // left with the best options for this turn.
    for (AttackPairs& pairing : pairings) {
        SimulationState state = initial;

        simulateCombatEvent(state, pairing);
        queueNewState(queue, state);

        // If the initial state has no attacks, then this is the first turn. Hence, we
        // store the attack list so we can actually follow through with the attack if this
        // particular simulation turns out to be ideal.
        if (state.firstAttacks.self.size() == 0) {
            state.firstAttacks = std::move(pairing);
        }
    }
}

AttackPairs runCombatSimulation(bw::Unitset selfUnits, bw::Unitset enemyUnits) {
    // The initial state of the simulation just starts out with the list of units that
    // we're provided to begin with.
    SimulationState initial;
    initial.selfUnits = std::move(selfUnits);
    initial.enemyUnits = std::move(enemyUnits);

    // The priority queue for the first step only contains the initial state.
    std::vector<SimulationState> initialQueue = { initial };

    for (int i = 0; i < MAX_TURNS; i++) {
        // For every turn of the simulation, we create a new priority queue to hold the
        // best states from this turn.
        std::vector<SimulationState> nextQueue;

        // For each of the existing states from the last turn, simulate them and add them
        // to the new queue.
        for (SimulationState& state : initialQueue) {
            simulateCombatTurn(nextQueue, state);
        }

        // This new queue becomes the initial queue for the following turn.
        initialQueue = std::move(nextQueue);
    }

    // Now that we have a priority queue of all the best states from a number of steps
    // later in the game, we can choose the best one and return the attack pairings that
    // the bot should take for this frame.
    SimulationState& best = findBestState(initialQueue);
    return best.firstAttacks;
}