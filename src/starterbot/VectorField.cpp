#include "VectorField.h"
#include "Tools.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <array>

/*   BWAPI GRID TYPES
 *   Pixel Level  (1x1 pixel)     bw::Position
 *   Walk Tile    (8x8 pixels)    bw::WalkPosition
 *   Build Tile   (32x32 pixels)  bw::TilePosition
 */

// constructor for Vector
Vector::Vector(float x_, float y_) : bw::Point<float, 1>(x_, y_) {}

// constructor for VectorField
VectorField::VectorField() {}

void VectorField::onStart() {

    // width and height in terms of WalkPosition; mapWidth and mapHeight return values in terms of TilePosition
    m_width = bw::Broodwar->mapWidth() * 4;
    m_height = bw::Broodwar->mapHeight() * 4;

    m_walkable = Grid<bool>(m_width, m_height, false);
    m_groundField = Grid<Vector>(m_width, m_height, { 1.0f, 1.0f });

    // Set the boolean grid data from the Map
    for (int x(0); x < m_width; ++x) {
        for (int y(0); y < m_height; ++y) {
            m_walkable.set(x, y, g_game->isWalkable(x * 8, y * 8));
        }
    }

    for (auto& resource : g_game->getStaticNeutralUnits()) {
        const bw::WalkPosition walkTile = bw::WalkPosition(resource->getTilePosition()) + bw::WalkPosition{ 2, 2 };

        for (int x = walkTile.x; x < walkTile.x + resource->getType().tileWidth() * 4; x += 8) {
            for (int y = walkTile.y; y < walkTile.y + resource->getType().tileHeight() * 4; y += 8) {
                m_walkable.set(x / 8, y / 8, false);
            }
        }
    }
}

void VectorField::onFrame() {
    if (m_drawField) {
        draw();
    }
}

void VectorField::draw() const {
    const bw::WalkPosition screen(g_game->getScreenPosition());
    const int sx = screen.x;
    const int sy = screen.y;
    const int ex = sx + 20 * 4;
    const int ey = sy + 15 * 4;

    for (int x = sx; x < ex; ++x) {
        for (int y = sy; y < ey; y++) {

            const bw::WalkPosition walkPos(x, y);
            if (!walkPos.isValid()) { continue; }

            const bw::Position start(walkPos);
            const bw::Position end = start + m_groundField.get(x, y);

            bw::Color vectorColor = m_walkable.get(x / 8, y / 8) ? bw::Colors::Green : bw::Colors::Red;
            g_game->drawLineMap(start, end, vectorColor);
        }
    }

    return;
}