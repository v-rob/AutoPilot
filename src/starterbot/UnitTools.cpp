#include "UnitTools.h"

static int getSquaredDistance(bw::Position a, bw::Position b) {
    bw::Position diff = a - b;
    return diff.x * diff.x + diff.y * diff.y;
}

struct ClusterDistance {
    Cluster* cluster;
    int distance;
};

static ClusterDistance getClosestCentroid(
        std::vector<Cluster>& clusters, bw::Unit unit, int maxSize, int maxIndex) {
    // Initially set the minimum distance to the maximal integer value, which any centroid
    // will match as closer than.
    Cluster* minCluster = nullptr;
    int minDistance = INT_MAX;

    for (int index = 0; index < maxIndex; index++) {
        Cluster& cluster = clusters[index];

        // If this cluster is already full, we can't assign any units to it, so move on.
        if (cluster.units.size() >= maxSize) {
            continue;
        }

        // Get the distance between this unit and the centroid of the cluster we're
        // looking at. If it's smaller than any distance we've found before, select this
        // as the closest cluster so far.
        int distance = getSquaredDistance(cluster.centroid, unit->getPosition());

        if (distance < minDistance) {
            minCluster = &cluster;
            minDistance = distance;
        }
    }

    return { minCluster, minDistance };
}

static ClusterDistance getClosestCentroid(
        std::vector<Cluster>& clusters, bw::Unit unit, int maxSize) {
    // Usually, we want the closest distance to any cluster, not a subset of the clusters.
    return getClosestCentroid(clusters, unit, maxSize, (int)clusters.size());
}

static void chooseInitialCentroids(
        std::vector<Cluster>& clusters, const bw::Unitset& units, int maxSize) {
    // We use a somewhat modified version of the k-means++ choice of initial centroids:
    // instead of randomly choosing a position with probability proportional to the
    // squared distance to the nearest centroid, we deterministically choose the position
    // with the largest such distance. This keeps our clusterings stable across frames.
    for (int index = 0; index < clusters.size(); index++) {
        // Set the maximal distance to the minimal integer value, which every distance
        // will be greater than.
        bw::Position maxPosition = bw::Positions::Origin;
        int maxDistance = INT_MIN;

        for (bw::Unit unit : units) {
            // Find the distance from this unit to the nearest centroid. If we don't have
            // any centroids yet, choose the unit that is farthest away from the origin.
            int distance;
            if (index == 0) {
                distance = getSquaredDistance(unit->getPosition(), bw::Positions::Origin);
            } else {
                distance = getClosestCentroid(clusters, unit, maxSize, index).distance;
            }

            // If this distance is larger than any one we've seen before, choose it as our
            // best initial guess for a centroid so far.
            if (distance > maxDistance) {
                maxPosition = unit->getPosition();
                maxDistance = distance;
            }
        }

        clusters[index].centroid = maxPosition;
    }
}

struct UnitDistance {
    bw::Unit unit;
    ClusterDistance closest;
};

static bool compareDistances(const UnitDistance& left, const UnitDistance& right) {
    // Compare the distances directly. Strangely, the STL uses a less than comparison to
    // implement a max heap, so we have to use a greater than comparison for a min heap.
    return left.closest.distance > right.closest.distance;
}

static void repopulateClusters(
        std::vector<Cluster>& clusters, const bw::Unitset& units, int maxSize) {
    // First, we clear out our old clusters so we can regenerate them.
    for (Cluster& cluster : clusters) {
        cluster.units.clear();
    }

    // Compute the distance between each unit and its closest centroid and insert these
    // into a vector that will be made into a priority queue.
    std::vector<UnitDistance> queue;
    for (bw::Unit unit : units) {
        queue.push_back({ unit, getClosestCentroid(clusters, unit, maxSize) });
    }

    std::make_heap(queue.begin(), queue.end(), &compareDistances);

    // As long as there are units in the queue, draw them out so they can be added to the
    // appropriate cluster.
    while (!queue.empty()) {
        // Pop the unit with the closest distance off the priority queue.
        std::pop_heap(queue.begin(), queue.end(), &compareDistances);

        UnitDistance dist = queue.back();
        queue.pop_back();

        if (dist.closest.cluster->units.size() < maxSize) {
            // If the cluster this unit has been assigned to has not been filled yet, add
            // the unit to the cluster.
            dist.closest.cluster->units.insert(dist.unit);
        } else {
            // Otherwise, we need to recompute which cluster this unit should be assigned
            // to. After doing so, push the unit back into the priority queue so that
            // units with a closer distance have a chance to be added to a cluster first.
            dist.closest = getClosestCentroid(clusters, dist.unit, maxSize);

            queue.push_back(dist);
            std::push_heap(queue.begin(), queue.end(), &compareDistances);
        }
    }
}

bw::Position findCentroid(const bw::Unitset& units) {
    // Compute the centroid of this cluster by averaging the positions of all the units.
    bw::Position total = bw::Positions::Origin;

    for (bw::Unit unit : units) {
        total += unit->getPosition();
    }

    // If the cluster is empty, be careful not to perform a division by zero.
    return total / std::max((int)units.size(), 1);
}

static void computeNewCentroids(std::vector<Cluster>& clusters) {
    // All the clusters have been populated, so we need to recompute the centroids of
    // these clusters. We do so by averaging the positions of all the units.
    for (Cluster& cluster : clusters) {
        cluster.centroid = findCentroid(cluster.units);
    }
}

std::vector<Cluster> findUnitClusters(const bw::Unitset& units, int desiredSize, int maxSize) {
    // We could iterate the k-means++ algorithm until we converge to a set of clusters,
    // but that's really unnecessary for our purposes. Instead, iterate a fixed number of
    // times, which gives us a good enough clustering.
    constexpr int MAX_ITER = 5;

    std::vector<Cluster> clusters;

    // Choose an appropriate amount of clusters given the desired average cluster size.
    clusters.resize((units.size() / desiredSize) + 1);

    // We choose a set of initial centroid for our algorithm to work with. Having a smart
    // initial guess is critical to finding a good clustering.
    chooseInitialCentroids(clusters, units, maxSize);

    // Now, we iterate the algorithm for our fixed number of steps, repopulating the
    // cluster sets each iteration and choosing new centroids based on those clusters.
    for (int iter = 0; iter < MAX_ITER; iter++) {
        repopulateClusters(clusters, units, maxSize);
        computeNewCentroids(clusters);
    }

    return clusters;
}