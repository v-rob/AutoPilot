#include "BaseManager.h"

BaseManager::BaseManager(UnitManager& unitManager) :
    m_unitManager(unitManager) {
}

void BaseManager::onStart() {
    for (bw::Unit resource : g_game->getStaticNeutralUnits()) {

        bool placed = false;

        for (bw::Unitset base : m_baseLocations) {
            if (resource->getTilePosition().getApproxDistance(bw::TilePosition(base.getPosition())) < RESOURCE_RADIUS) {
                base.insert(resource);
                placed = true;
                break;
            }
        }

        if (placed) {
            continue;
        }

        bw::Unitset set;
        set.insert(resource);
        m_baseLocations.push_back(set);
    }

    for (auto& base : m_baseLocations) {
        bw::Position centroid = base.getPosition();
        std::cout << centroid << std::endl;
    }
}

void BaseManager::onDraw() {
    for (auto& base : m_baseLocations) {
        bw::Position centroid = base.getPosition();
        for (auto& unit : base) {
            g_game->drawLineMap(centroid, unit->getPosition(), bw::Colors::Green);
        }
    }
}