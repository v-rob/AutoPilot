#define _USE_MATH_DEFINES

#include "VectorField.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <array>
#include <algorithm>

/*   BWAPI GRID TYPES
 *   Pixel Level  (1x1 pixel)     bw::Position
 *   Walk Tile    (8x8 pixels)    bw::WalkPosition
 *   Build Tile   (32x32 pixels)  bw::TilePosition
 */


double util::boundAngle(double angle) {
    if (angle >= M_PI * 2) { return angle - M_PI * 2; }
    if (angle < 0) { return M_PI * 2 + angle; }
    return angle;
}

double util::angleBetween(const Vector& point1, const Vector& point2) {
    const Vector direction = point2 - point1;
    return boundAngle(std::atan2(direction.y, direction.x));
}

float util::distanceBetween(const Vector& point1, const Vector& point2) {
    float dx = static_cast<float>(point2.x - point1.x);
    float dy = static_cast<float>(point2.y - point1.y);
    return std::sqrt(dx * dx + dy * dy);
}


// constructors for Vector
Vector::Vector(float x_, float y_) : bw::Point<float, 1>(x_, y_) {}
Vector::Vector(int x_, int y_) : bw::Point<float, 1>(static_cast<float>(x_), static_cast<float>(y_)) {}
Vector::Vector(const bw::Point<float, 1>& p) : bw::Point<float, 1>(p) {}
Vector::Vector(const bw::Position& p) : bw::Point<float, 1>(p) {}
Vector::Vector(double angle) : bw::Point<float, 1>(std::cos(angle), std::sin(angle)) {}

float Vector::length() {
    //return std::sqrt(x * x + y * y);
    return util::distanceBetween({ 0.0f, 0.0f }, { x, y });
}

void Vector::normalize() {
    if (length() == 0) { return; }
    *this = *this / length();
}


// constructor for VectorField
VectorField::VectorField(UnitManager& unitManager) : m_unitManager(unitManager) {}

void VectorField::onStart() {

    bw::Position p1(0, 0);
    bw::Position p2(100, 0);

    std::cout << util::distanceBetween(p1, p2) << "\n";

    // width and height in terms of WalkPosition; mapWidth and mapHeight return values in terms of TilePosition
    m_width = bw::Broodwar->mapWidth() * 4;
    m_height = bw::Broodwar->mapHeight() * 4;

    m_walkable = Grid<char>(m_width, m_height, true);
    m_groundField = Grid<Vector>(m_width, m_height, { 0.0f, 0.0f });
    m_scoutField = Grid<Vector>(m_width, m_height, { 0.0f, 0.0f });

    // Set the boolean grid data from the map
    for (int x(0); x < m_width; ++x) {
        for (int y(0); y < m_height; ++y) {
            m_walkable.set(x, y, g_game->isWalkable(x, y));
        }
    }

    bool once = false;
    for (auto& resource : g_game->getStaticNeutralUnits()) {
        if (once) {
            break;
        }

        const bw::WalkPosition topLeft(resource->getTilePosition());
        const bw::WalkPosition bottomRight = bw::WalkPosition{
            topLeft.x + resource->getType().tileWidth() * 4,
            topLeft.y + resource->getType().tileHeight() * 4
        };

        for (int x = topLeft.x; x < topLeft.x + resource->getType().tileWidth() * 4; x++) {
            for (int y = topLeft.y; y < topLeft.y + resource->getType().tileHeight() * 4; y++) {
                m_walkable.set(x, y, false);
                //std::cout << "x, y: " << x << ", " << y << "\n";
            }
        }

        //const bw::WalkPosition margin{ 2, 2 };
        updateVectorRegion(topLeft, bottomRight, 8);

        //once = true;
    }

    /* ALIVE BUILDINGS are handled in onFrame.*/
    std::cout << "Alive Buildings are initially empty, will fill in as needed\n" << "";
}


void VectorField::onFrame() {
    //The set difference implementation should be called in here (THIS IS CAUSING THE ISSUE)
    bw::Unitset m_difference;
    //Unitset doesn't support .start() or .end()

    /*Check Add Building(s)*/
    for (bw::Unit shadowBuilding : m_unitManager.shadowUnits(bw::Filter::IsBuilding)) {
        bool difference = true;
        for (bw::Unit aliveBuilding : m_aliveBuildings) {
            if (shadowBuilding != aliveBuilding) {
                continue;
            }
            else {
                difference = false; //we found shadow in alive
            }
        }
        if (difference == true){
            m_aliveBuildings.insert(shadowBuilding);
            m_difference.insert(shadowBuilding); //either add invalid or valid squares
            drawBuildingTile(shadowBuilding);
        }
    }

    /*Check Remove Building(s)*/
    for (bw::Unit aliveBuilding : m_aliveBuildings) {
        //std::cout << "" << (aliveBuilding) << "";
        bool difference = true;
        for (bw::Unit shadowBuilding : m_unitManager.shadowUnits(bw::Filter::IsBuilding)) {
            if (aliveBuilding != shadowBuilding) {
                continue;
            }
            else {
                difference = false; //we found alive in shadow
            }
        }
        if (difference == true) {
            m_aliveBuildings.erase(aliveBuilding);
            m_difference.insert(aliveBuilding);
            drawFreeBuildingTile(aliveBuilding);
        }
    }
    

    //Whenever a building is added or removed from map, update the following Tile(s)
    //if (!m_difference.empty()) {
    //    //std::cout << "Building Difference" << "";
    //    for (auto& building : m_difference) {
    //        drawBuildingTile(building);
    //        //std::cout << "{" << building << "}";
    //    }
    //}
    //e.g. shadowunits = [1,2,3], m_aliveBuildings = [1,2,3] no change from VectorField::onStart
    // e.g. shadowunits = [1,2,3,4], m_aliveBuildings = [1,2,3] building added
    /* m_difference = [4] */

    if (m_drawField) {
        draw();
    }

    m_mouse = g_game->getMousePosition() + g_game->getScreenPosition();
    bw::WalkPosition walkMouse(m_mouse);


    for (auto& tile : m_mouseTiles) {
        m_scoutField.set(tile.x, tile.y, { 0.0f, 0.0f });
    }

    m_mouseTiles.clear();

    const int radius = 8;
    for (int x = walkMouse.x - radius; x <= walkMouse.x + radius; x++) {
        for (int y = walkMouse.y - radius; y <= walkMouse.y + radius; y++) {
            bw::WalkPosition walkTile(x, y);

            if (!walkTile.isValid()) { continue; }

            bw::Position tileCenter = bw::Position(walkTile) + bw::Position{ 4, 4 };
            float distance = util::distanceBetween(m_mouse, tileCenter);

            if (distance > radius * 8.0f) { continue; }

            //drawWalkTile(walkTile, bw::Colors::Orange);
            Vector vec(util::angleBetween(m_mouse, tileCenter));
            vec *= 1.2 - (distance / (radius * 8.0f));

            m_scoutField.set(x, y, vec);
            m_mouseTiles.push_back(walkTile);
        }
    }
}

void VectorField::updateVectorRegion(bw::WalkPosition topLeft, bw::WalkPosition bottomRight, int margin) {
    const int sx = topLeft.x - margin;
    const int sy = topLeft.y - margin;
    const int ex = bottomRight.x + margin;
    const int ey = bottomRight.y + margin;

    const int mooreRadius = margin;

    for (int x = sx; x < ex; x++) {
        for (int y = sy; y < ey; y++) {
            bw::WalkPosition walkTile(x, y);

            if (!bw::WalkPosition{ x, y }.isValid()) { continue; }

            if (!m_walkable.get(x, y)) { continue; }

            Vector centroid{ 0.0f, 0.0f };
            int filledCount = 0;

            for (int rx = -mooreRadius; rx <= mooreRadius; rx++) {
                for (int ry = -mooreRadius; ry <= mooreRadius; ry++) {
                    const int wx = x + rx;
                    const int wy = y + ry;

                    if (!((bw::WalkPosition{ wx, wy }).isValid())) { 
                        continue; 
                    }

                    if (!m_walkable.get(wx, wy)) {
                        centroid += Vector{ rx, ry };
                        filledCount++;
                    }
                }
            }

            if (filledCount == 0) {
                m_groundField.set(x, y, Vector{ 0.0f, 0.0f });
                continue;
            }

            bw::Position pos(walkTile);
            centroid = centroid / filledCount + Vector{ pos };

            double angle = util::angleBetween(centroid, pos);
            float distance = util::distanceBetween(centroid, pos);

            if (distance > mooreRadius) { continue; }

            Vector vec = Vector{ angle } * (1.3 - (distance / mooreRadius));
            m_groundField.set(x, y, vec);

        }
    }
}

void VectorField::onSendText(const std::string& text) {
    if (text == "/pause") {
        g_game->pauseGame();
    }

    if (text == "/field") {
        m_drawField = !m_drawField;
    }
};

void VectorField::draw() const {
    const bw::WalkPosition screen(g_game->getScreenPosition());
    const int sx = screen.x;
    const int sy = screen.y;
    const int ex = sx + 20 * 4; // Screen width in walk tiles
    const int ey = sy + 15 * 4; // screen height in walk tiles

    for (int x = sx; x < ex; ++x) {
        for (int y = sy; y < ey; y++) {

            bw::WalkPosition walkTile(x, y);

            if (!walkTile.isValid()) { continue; }

            //bw::Color tileColor = m_walkable.get(x, y) ? bw::Colors::Green : bw::Colors::Red;

            if (m_walkable.get(x, y)) {
                Vector ground_vector = m_groundField.get(x, y);
                Vector scout_vector = m_scoutField.get(x, y);
                Vector vector = ground_vector + scout_vector;

                drawWalkVector(walkTile, vector, bw::Colors::Green);
            } else {
                drawWalkTile(walkTile, bw::Colors::Red);
            }
        }
    }

    bw::Position pixelMouse = m_mouse;
    bw::WalkPosition walkMouse(m_mouse);
    bw::TilePosition tileMouse(m_mouse);

    drawWalkTile(walkMouse, bw::Colors::Cyan);
    //drawBuildTile(tileMouse, bw::Colors::Purple);

    const char white = '\x04';

    g_game->drawBoxScreen(0, 0, 180, 80, bw::Colors::Black, true);
    g_game->setTextSize(BWAPI::Text::Size::Huge);
    g_game->drawTextScreen(10, 5, "%cMouse Position", '\x04');
    g_game->setTextSize(BWAPI::Text::Size::Default);
    g_game->drawTextScreen(10, 30, "%cPixel Position: (%d, %d)", '\x04', pixelMouse.x, pixelMouse.y);
    g_game->drawTextScreen(10, 45, "%cWalk Position: (%d, %d)", '\x04', walkMouse.x, walkMouse.y);
    g_game->drawTextScreen(10, 60, "%cTile Position:  (%d, %d)", '\x04', tileMouse.x, tileMouse.y);
}

void VectorField::drawWalkTile(bw::WalkPosition walkTile, bw::Color color) const {
    const int padding = 1;
    const int px = walkTile.x * 8 + padding;
    const int py = walkTile.y * 8 + padding;
    const int d = 8 - 2 * padding;

    g_game->drawLineMap(px, py, px + d, py, color);
    g_game->drawLineMap(px + d, py, px + d, py + d, color);
    g_game->drawLineMap(px + d, py + d, px, py + d, color);
    g_game->drawLineMap(px, py + d, px, py, color);
}

void VectorField::drawBuildTile(bw::TilePosition buildTile, bw::Color color) const {
    const int padding = 1;
    const int px = buildTile.x * 32 + padding;
    const int py = buildTile.y * 32 + padding;
    const int d = 32 - 2 * padding;

    g_game->drawLineMap(px, py, px + d, py, color);
    g_game->drawLineMap(px + d, py, px + d, py + d, color);
    g_game->drawLineMap(px + d, py + d, px, py + d, color);
    g_game->drawLineMap(px, py + d, px, py, color);
}

void VectorField::drawWalkVector(bw::WalkPosition walkTile, Vector vector, bw::Color color) const {
    const int length = 6;
    const bw::Position tail = bw::Position(walkTile) + bw::Position{ 4, 4 };
    const bw::Position head = tail + vector * length;

    g_game->drawLineMap(tail, head, color);
}

void VectorField::drawBuildingTile(bw::Unit building) { //add another parameter later
    const bw::WalkPosition walkTile(building->getTilePosition());
    for (int x = walkTile.x; x < walkTile.x + building->getType().tileWidth() * 4; x++) {
        for (int y = walkTile.y; y < walkTile.y + building->getType().tileHeight() * 4; y++) {
            m_walkable.set(x, y, false);
        }
    }
}

void VectorField::drawFreeBuildingTile(bw::Unit building) {
    const bw::WalkPosition walkTile(building->getTilePosition());
    for (int x = walkTile.x; x < walkTile.x + building->getType().tileWidth() * 4; x++) {
        for (int y = walkTile.y; y < walkTile.y + building->getType().tileHeight() * 4; y++) {
            m_walkable.set(x, y, true);
        }
    }
}
