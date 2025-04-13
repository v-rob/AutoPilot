#include "BaseManager.h"

BaseManager::BaseManager(UnitManager& unitManager) :
    m_unitManager(unitManager) {
}

Base& BaseManager::getClosestBase(bw::Unit& unit) {
    Base* closestBase;
    int shortestDistance = INT_MAX;

    // getDistance uses Brood War's built-in pathfinding, so this will find the
    // base that's closest while accounting for terrain.
    for (Base& base : m_bases) {
        int distance = unit->getDistance(base.centroid);

        if (distance < shortestDistance) {
            shortestDistance = distance;
            closestBase = &base;
        }
    }

    return *closestBase;
}

void BaseManager::onStart() {

    std::vector<bw::Unitset> resourceClusters;

    // Cluster all resources on the map.
    for (bw::Unit unit : g_game->getStaticNeutralUnits()) {

        // Ignore non-resources (special buildings that exist at game-start).
        if (!(unit->getType().isMineralField() || unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser)) {
            continue;
        }

        bw::TilePosition resourcePosition = unit->getTilePosition();
        bool placed = false;

        for (bw::Unitset& cluster : resourceClusters) {
            if (resourcePosition.getApproxDistance(bw::TilePosition(cluster.getPosition())) < RESOURCE_RADIUS) {
                cluster.insert(unit);
                placed = true;
                break;
            }
        }

        if (!placed) {
            bw::Unitset newCluster;
            newCluster.insert(unit);
            resourceClusters.push_back(newCluster);
        }
    }

    for (auto& cluster : resourceClusters) {
        bw::Position centroid = cluster.getPosition();
        std::vector<bw::Unit> minerals;
        std::vector<bw::Unit> geysers;

        for (auto& resource : cluster) {
            if (resource->getType().isMineralField()) {
                minerals.push_back(resource);
            }
            if (resource->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
                geysers.push_back(resource);
            }
        }

        m_bases.push_back({ centroid, minerals, geysers, {}, 0 });
    }
}

void BaseManager::onFrame() {

    bw::Unitset shadowBuildings = m_unitManager.shadowUnits(
        bw::Filter::IsBuilding &&                                   // get enemy buildings
        !bw::Filter::IsSpecialBuilding && !bw::Filter::IsNeutral    // ignore resources and pre-exisiting buildings
    );

    // Check for new buildings.
    for (bw::Unit building : shadowBuildings) {
        if (m_buildings.find(building) == m_buildings.end()) {
            m_buildings.insert(building);

            // If there's a new building, find the base it now belongs to.
            Base& closestBase = getClosestBase(building);

            // Add the building to this base
            closestBase.buildings.push_back(building);

            // If the building belongs to the enemy, we increment the base's enemy count
            if (building->getPlayer() == g_game->enemy()) {
                closestBase.enemyCount += 1;
            }
        }
    }

    // Check for destroyed buildings.
    for (bw::Unit building : m_buildings) {
        if (shadowBuildings.find(building) == shadowBuildings.end()) {
            m_buildings.erase(building);

            // If a building has been destoryed, find the base it had belonged to.
            for (Base& base : m_bases) {
                auto it = std::find(base.buildings.begin(), base.buildings.end(), building);

                // Remove the building from this base
                if (it != base.buildings.end()) {
                    base.buildings.erase(it);

                    // If the building belonged to the enemy, we decrement the base's enemy count
                    if (building->getPlayer() == g_game->enemy()) {
                        base.enemyCount += 1;
                    }

                    break;
                }
            }
        }
    }
}

void BaseManager::onDraw() {
    if (!m_drawBases) {
        return;
    }

    for (auto& base : m_bases) {
        std::vector<bw::Unit> resources = base.minerals;
        resources.insert(resources.end(), base.geysers.begin(), base.geysers.end());

        for (auto& resource : resources) {
            g_game->drawLineMap(bw::Position(base.centroid), resource->getPosition(), bw::Colors::Green);
        }

        for (auto& building : base.buildings) {
            g_game->drawLineMap(bw::Position(base.centroid), building->getPosition(), bw::Colors::Purple);
        }
    }
}

void BaseManager::onSendText(const std::string& text) {
    if (text == "/drawBases") {
        m_drawBases = !m_drawBases;
    }
};