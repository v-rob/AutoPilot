#include "UnitTools.h"

int getSquaredDistance(bw::Position a, bw::Position b) {
    bw::Position diff = a - b;
    return diff.x * diff.x + diff.y * diff.y;
}

bool isInRadius(bw::Position a, bw::Position b, int radius) {
    return getSquaredDistance(a, b) <= radius * radius;
}

bool isInRectangle(bw::Position pos, bw::Position topLeft, bw::Position botRight) {
    return pos.x >= topLeft.x && pos.x < botRight.x && pos.y >= topLeft.y && pos.y < botRight.y;
}

bw::Unitset getUnitsInRadius(const bw::Unitset& units, bw::Position pos, int radius) {
    bw::Unitset found;

    for (bw::Unit unit : units) {
        if (isInRadius(unit->getPosition(), pos, radius)) {
            found.insert(unit);
        }
    }

    return found;
}

bool hasUnitInRadius(const bw::Unitset& units, bw::Position pos, int radius) {
    for (bw::Unit unit : units) {
        if (isInRadius(unit->getPosition(), pos, radius)) {
            return true;
        }
    }

    return false;
}

bw::Unitset getUnitsInRectangle(
        const bw::Unitset& units, bw::Position topLeft, bw::Position botRight) {
    bw::Unitset found;

    for (bw::Unit unit : units) {
        if (isInRectangle(unit->getPosition(), topLeft, botRight)) {
            found.insert(unit);
        }
    }

    return found;
}

bool hasUnitInRectangle(const bw::Unitset& units, bw::Position topLeft, bw::Position botRight) {
    for (bw::Unit unit : units) {
        if (isInRectangle(unit->getPosition(), topLeft, botRight)) {
            return true;
        }
    }

    return false;
}

bw::Unit getClosestUnit(const bw::Unitset& units, bw::Position pos) {
    bw::Unit minUnit = nullptr;
    int minDistance = INT_MAX;

    for (bw::Unit unit : units) {
        int distance = getSquaredDistance(pos, unit->getPosition());

        if (distance < minDistance) {
            minUnit = unit;
            minDistance = distance;
        }
    }

    return minUnit;
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

static void computeNewCentroids(std::vector<Cluster>& clusters) {
    // All the clusters have been populated, so we need to recompute the centroids of
    // these clusters. We do so by averaging the positions of all the units.
    for (Cluster& cluster : clusters) {
        cluster.centroid = cluster.units.getPosition();
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

std::vector<Cluster> findRadialClusters(const bw::Unitset& units, int radius) {
    std::vector<Cluster> clusters;

    for (bw::Unit unit : units) {
        // For each unit, we need to add it to either an existing cluster or create a new
        // cluster to put it in.
        bool foundCluster = false;

        for (Cluster& cluster : clusters) {
            // We can only add this unit to the cluster if the distance between it and
            // every other unit in the cluster is within the given radius.
            bool inRadius = true;

            for (bw::Unit other : cluster.units) {
                // Since the radius is specified from the centroid of the circle, we need
                // to multiply it by two to get the diameter for our distance comparisons.
                if (!isInRadius(unit->getPosition(), other->getPosition(), radius * 2)) {
                    inRadius = false;
                    break;
                }
            }

            // If we're within the radius for this cluster, we can add the unit to it.
            if (inRadius) {
                cluster.units.insert(unit);
                foundCluster = true;
                break;
            }
        }

        // Otherwise, our unit couldn't be placed into any existing cluster, so make a new
        // cluster for just this unit.
        if (!foundCluster) {
            Cluster cluster;
            cluster.units.insert(unit);
            clusters.push_back(std::move(cluster));
        }
    }

    // We didn't need to compute centroid for the purposes of this clustering algorithm,
    // so compute them all manually now that all the units have been added.
    for (Cluster& cluster : clusters) {
        cluster.centroid = cluster.units.getPosition();
    }

    return clusters;
}