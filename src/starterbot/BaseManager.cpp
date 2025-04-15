#include "BaseManager.h"
#include "UnitTools.h"

BaseManager::BaseManager(UnitManager& unitManager) :
    m_unitManager(unitManager) {
}

Base& BaseManager::getClosestBase(bw::Unit unit) {
    Base* closestBase;
    int shortestDistance = INT_MAX;

    for (Base& base : m_bases) {
        int distance = unit->getDistance(base.centroid);

        if (distance < shortestDistance) {
            shortestDistance = distance;
            closestBase = &base;
        }
    }

    return *closestBase;
}

// Perform basic map analysis by finding all resources on the map, clustering them, and 
// creating bases at them.
void BaseManager::onStart() {

    m_buildings.clear();
    m_bases.clear();

    // Get all resources, i.e. all minerals and geysers.
    bw::Unitset resources = g_game->getMinerals();
    bw::Unitset geysers = g_game->getGeysers();
    resources.insert(geysers.begin(), geysers.end());

    std::vector<Cluster> resourceClusters = findRadialClusters(resources, RESOURCE_RADIUS);

    // For every resource cluster found, create a new base.
    for (auto& cluster : resourceClusters) {
        // The set of buildings and enemyCount initalize as empty and as zero, respectively.
        m_bases.push_back({ cluster.centroid, cluster.units, {}, 0 });
    }
}

// Each frame, check if there is a difference in buildings, i.e. any have been created or
// destroyed. If there is, update the bases.
void BaseManager::onFrame() {

    bw::Unitset shadowBuildings = m_unitManager.shadowUnits(
        bw::Filter::IsBuilding &&                                   // get all buildings
        !bw::Filter::IsSpecialBuilding && !bw::Filter::IsNeutral    // ignore pre-exisiting buildings and resources
    );
    
    // There have been no buildings created or destroyed.
    if (shadowBuildings == m_buildings) {
        return;
    }

    // Otherwise, recompute the bases.
    for (Base base : m_bases) {
        base.enemyCount = 0;
        base.buildings.clear();
    }

    for (bw::Unit building : shadowBuildings) {
        // Find the base this building belongs to and add it
        Base& closestBase = getClosestBase(building);
        closestBase.buildings.insert(building);

        // If the building belongs to the enemy, we increment the base's enemy count
        if (building->getPlayer() == g_game->enemy()) {
            closestBase.enemyCount += 1;
        }
    }

    // Update the set of building units so we can check again next frame
    m_buildings = shadowBuildings;
}

void BaseManager::onDraw() {
    if (!m_drawBases) {
        return;
    }

    // Draw lines from each resource / building to each base's centroid
    for (auto& base : m_bases) {
        for (auto& resource : base.resources) {
            g_game->drawLineMap(bw::Position(base.centroid), resource->getPosition(), bw::Colors::Orange);
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