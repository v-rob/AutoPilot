#define _USE_MATH_DEFINES

#include "VectorField.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <array>
#include <algorithm>
#include <optional>

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

template <typename T>
double util::angleBetween(const T& point1, const T& point2) {
    const T direction = point2 - point1;
    return boundAngle(std::atan2(direction.y, direction.x));
}

float util::distanceBetween(const Vector2& point1, const Vector2& point2) {
    float dx = static_cast<float>(point2.x - point1.x);
    float dy = static_cast<float>(point2.y - point1.y);
    return std::sqrt(dx * dx + dy * dy);
}

std::vector<bw::Position> util::convexHull(std::vector<bw::Position> points) {
    // https://www.geeksforgeeks.org/convex-hull-using-jarvis-algorithm-or-wrapping/

    // To find orientation of ordered triplet (p, q, r)
    // 0 -> p, q and r are collinear
    // 1 -> Clockwise
    // 2 -> Counterclockwise
    auto orientation = [](bw::Position p, bw::Position q, bw::Position r) {
        int val = (q.y - p.y) * (r.x - q.x) -
            (q.x - p.x) * (r.y - q.y);

        if (val == 0) { return 0; } // collinear
        return (val > 0) ? 1 : 2;   // clock or counterclock wise
    };

    int n = points.size();

    if (n < 3) return {};

    std::vector<bw::Position> hull;

    int l = 0;
    for (int i = 1; i < n; i++) {
        if (points[i].x < points[l].x) {
            l = i;
        }
    }

    int p = l, q;
    do {
        hull.push_back(points[p]);

        q = (p + 1) % n;
        for (int i = 0; i < n; i++) {
            if (orientation(points[p], points[i], points[q]) == 2) {
                q = i;
            }
        }

        p = q;

    } while (p != l);

    return hull;
}

bw::Position util::closestPointOnSegment(bw::Position point, std::pair<bw::Position, bw::Position> segment) {
    // https://stackoverflow.com/a/6853926/27539798

    float A = point.x - segment.first.x;
    float B = point.y - segment.first.y;
    float C = segment.second.x - segment.first.x;
    float D = segment.second.y - segment.first.y;

    float dot = A * C + B * D;
    float len_sq = C * C + D * D;
    float param = -1;

    if (len_sq != 0) { //in case of 0 length line
        param = dot / len_sq;
    }

    bw::Position closest;

    if (param < 0) {
        closest = segment.first;
    } else if (param > 1) {
        closest = segment.second;
    } else {
        closest = segment.first + bw::Position(param * C, param * D);
    }

    return closest;
}


// constructors for Vector2
Vector2::Vector2(float x_, float y_) : bw::Point<float, 1>(x_, y_) {}
Vector2::Vector2(int x_, int y_) : bw::Point<float, 1>(static_cast<float>(x_), static_cast<float>(y_)) {}
Vector2::Vector2(double x_, double y_) : bw::Point<float, 1>(static_cast<float>(x_), static_cast<float>(y_)) {}
Vector2::Vector2(const bw::Point<float, 1>& p) : bw::Point<float, 1>(p) {}
Vector2::Vector2(const bw::Position& p) : bw::Point<float, 1>(p) {}
Vector2::Vector2(double angle) : bw::Point<float, 1>(std::cos(angle), std::sin(angle)) {}

float Vector2::length() {
    return util::distanceBetween({ 0.0f, 0.0f }, { x, y });
}

Vector2 Vector2::normalize() {
    if (length() == 0) { return *this; }
    *this = *this / length();
    return *this;
}

Vector2 Vector2::rotate(double angle) {
    *this = Vector2(std::cos(angle), std::sin(angle)) * x + Vector2(-std::sin(angle), std::cos(angle)) * y;
    return *this;
}


// constructor for VectorField
VectorField::VectorField(UnitManager& unitManager) : m_unitManager(unitManager) {}

void VectorField::onStart() {

    m_enemyBuildings.clear();

    // width and height in terms of WalkPosition; mapWidth and mapHeight return values in terms of TilePosition
    m_width = bw::Broodwar->mapWidth() * WALKPOS_PER_TILEPOS;
    m_height = bw::Broodwar->mapHeight() * WALKPOS_PER_TILEPOS;

    m_walkable = Grid<char>(m_width, m_height, true);

    m_pathField   = Grid<std::optional<Vector2>>(m_width, m_height, std::nullopt);
    m_groundField = Grid<std::optional<Vector2>>(m_width, m_height, std::nullopt);
    m_enemyField  = Grid<std::optional<Vector2>>(m_width, m_height, Vector2(0.0f, 0.0f));

    // Initialize the vectors around each potential starting location
    for (bw::TilePosition pos : g_game->getStartLocations()) {
        const bw::WalkPosition center = bw::WalkPosition(pos);
        const bw::Position v_center(center);

        for (int x = center.x - BASE_WIDTH / 2; x < center.x + BASE_WIDTH / 2; x++) {
            for (int y = center.y - BASE_WIDTH / 2; y < center.y + BASE_WIDTH / 2; y++) {
                if (!bw::WalkPosition(x, y).isValid()) { continue; }

                const bw::Position v_tile(bw::WalkPosition(x, y));
                double angle = util::angleBetween(v_tile, v_center);

                //m_groundField.set(x, y, Vector2(angle));
                m_groundField.set(x, y, Vector2(0, 0));
            }
        }
    }

    // Set the boolean grid data from the map
    for (int x(0); x < m_width; ++x) {
        for (int y(0); y < m_height; ++y) {
            m_walkable.set(x, y, g_game->isWalkable(x, y));
        }
    }

    for (auto& resource : g_game->getStaticNeutralUnits()) {
        updateWalkable(resource, false);
        updateGroundField(resource, BUILDING_MARGIN);
    }

    generatePath();
}


void VectorField::onFrame() {

    bw::Unitset enemyShadowBuildings = m_unitManager.shadowUnits(
        bw::Filter::IsBuilding && bw::Filter::IsEnemy &&            // get all enemy buildings
        !bw::Filter::IsSpecialBuilding && !bw::Filter::IsNeutral    // ignore resources and pre-exisiting buildings
    );
    bool difference = false;

    // Check for new buildings
    for (bw::Unit building : enemyShadowBuildings) {
        if (m_enemyBuildings.find(building) == m_enemyBuildings.end()) {
            m_enemyBuildings.insert(building);
            updateWalkable(building, false);
            updateGroundField(building, BUILDING_MARGIN);
            difference = true;
        }
    }

    // Check for destroyed buildings
    for (bw::Unit building : m_enemyBuildings) {
        if (enemyShadowBuildings.find(building) == m_enemyBuildings.end()) {
            m_enemyBuildings.erase(building);
            updateWalkable(building, false);
            difference = true;
        }
    }

    // If at least one building has been created or destroyed, regenerate the path
    if (difference) {
        generatePath();
        updatePathField();
    }

    // Reset the field for enemy troops
    // TODO: instead of reinstantializing grid, use setAll (weird behavior with std::nullopt)
    m_enemyField = Grid<std::optional<Vector2>>(m_width, m_height, Vector2(0.0f, 0.0f));

    // Point vectors away from each mobile enemy troop on the enemy field "layer"
    for (auto& troop : m_unitManager.shadowUnits(bw::Filter::CanMove && bw::Filter::IsEnemy)) {
        updateEnemyField(bw::WalkPosition(troop->getPosition()));
    }

    m_mouse = g_game->getMousePosition() + g_game->getScreenPosition();
    updateEnemyField(bw::WalkPosition(m_mouse));

    if (m_drawField) {
        draw();
    }
}

void VectorField::generatePath() {
    m_path.clear();

    std::vector<bw::Position> points;
    for (auto& building : m_enemyBuildings) {
        points.push_back(building->getPosition());
    }

    // get the convex hull (i.e. outermost points) of all enemy buildings
    std::vector<bw::Position> hull = util::convexHull(points);
    int radius = m_scoutType.sightRange() / 2;

    // find the tangent lines between each sucessive circle in the hull
    // TODO: find the arcs between each tangent
    for (int i = 0; i < hull.size(); i++) {
        bw::Position current = hull.at(i);
        bw::Position next = hull.at((i + 1) % hull.size());

        Vector2 direction = next - current;
        Vector2 perpendicular = direction.normalize().rotate(-M_PI / 2) * radius;
        m_path.push_back(bw::WalkPosition(current + perpendicular));
        m_path.push_back(bw::WalkPosition(next + perpendicular));
    }
}

// TODO: use util::closestPointOnSegment to orient vectors towards path
void VectorField::updatePathField() {}

// TODO: refactor to extract common functionality between updateGroundField and updateEnemyField
void VectorField::updateGroundField(bw::Unit unit, int margin) {
    const bw::WalkPosition position(unit->getTilePosition());

    const int sx = position.x - margin;
    const int sy = position.y - margin;
    const int ex = position.x + unit->getType().tileWidth() * WALKPOS_PER_TILEPOS + margin;
    const int ey = position.y + unit->getType().tileHeight() * WALKPOS_PER_TILEPOS + margin;

    const int mooreRadius = margin;

    for (int x = sx; x < ex; x++) {
        for (int y = sy; y < ey; y++) {
            bw::WalkPosition walkTile(x, y);

            if (!bw::WalkPosition{ x, y }.isValid()) { continue; }

            if (!m_walkable.get(x, y)) { continue; }

            Vector2 centroid{ 0.0f, 0.0f };
            int filledCount = 0;

            for (int rx = -mooreRadius; rx <= mooreRadius; rx++) {
                for (int ry = -mooreRadius; ry <= mooreRadius; ry++) {
                    const int wx = x + rx;
                    const int wy = y + ry;

                    if (!((bw::WalkPosition{ wx, wy }).isValid())) { 
                        continue; 
                    }

                    if (!m_walkable.get(wx, wy)) {
                        centroid += Vector2{ rx, ry };
                        filledCount++;
                    }
                }
            }

            if (filledCount == 0) {
                m_groundField.set(x, y, Vector2{ 0.0f, 0.0f });
                continue;
            }

            bw::Position pos(walkTile);
            centroid = centroid / filledCount + Vector2{ pos };

            double angle = util::angleBetween<bw::Position>(centroid, pos);
            float distance = util::distanceBetween(centroid, pos);

            if (distance > mooreRadius) { continue; }

            Vector2 vec = Vector2{ angle } * (1.3 - (distance / mooreRadius));
            m_groundField.set(x, y, vec);
        }
    }
}

void VectorField::updateEnemyField(bw::WalkPosition position) {
    const int radius = 8;
    for (int x = position.x - radius; x <= position.x + radius; x++) {
        for (int y = position.y - radius; y <= position.y + radius; y++) {
            bw::WalkPosition walkTile(x, y);

            if (!walkTile.isValid()) { continue; }

            bw::Position tileCenter = bw::Position(walkTile) + bw::Position{ 4, 4 };
            float distance = util::distanceBetween(m_mouse, tileCenter);

            if (distance > radius * 8.0f) { continue; }

            Vector2 vec(util::angleBetween(m_mouse, tileCenter));
            vec *= 1.2 - (distance / (radius * 8.0f));

            m_enemyField.set(x, y, vec);
        }
    }
}

void VectorField::updateWalkable(bw::Unit unit, bool value) {
    const bw::WalkPosition walkTile(unit->getTilePosition());
    for (int x = walkTile.x; x < walkTile.x + unit->getType().tileWidth() * WALKPOS_PER_TILEPOS; x++) {
        for (int y = walkTile.y; y < walkTile.y + unit->getType().tileHeight() * WALKPOS_PER_TILEPOS; y++) {
            m_walkable.set(x, y, value);
        }
    }
}

std::optional<Vector2> VectorField::getVectorSum(int x, int y) const {
    std::optional<Vector2> ground_vector = m_groundField.get(x, y);
    std::optional<Vector2> enemy_vector = m_enemyField.get(x, y);

    if (ground_vector == std::nullopt || enemy_vector == std::nullopt) {
        return std::nullopt;
    }

    Vector2 vector = *ground_vector + *enemy_vector;
    return vector;
}

void VectorField::onSendText(const std::string& text) {
    if (text == "/pause") {
        g_game->pauseGame();
    }

    if (text == "/drawField") {
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
                std::optional<Vector2> vector = getVectorSum(x, y);

                if (vector == std::nullopt) {
                    continue;
                }

                drawWalkVector(walkTile, *vector, bw::Colors::Green);
            } else {
                drawWalkTile(walkTile, bw::Colors::Red);
            }
        }
    }

    for (auto& building : m_enemyBuildings) {
        g_game->drawCircleMap(building->getPosition(), m_scoutType.sightRange() / 2, bw::Colors::Cyan);
    }

    drawPolyLine(m_path, bw::Colors::Cyan);

    bw::Position pixelMouse = m_mouse;
    bw::WalkPosition walkMouse(m_mouse);
    bw::TilePosition tileMouse(m_mouse);

    drawWalkTile(walkMouse, bw::Colors::Purple);
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

void VectorField::drawWalkVector(bw::WalkPosition walkTile, Vector2 vector, bw::Color color) const {
    const int length = 6;
    const bw::Position tail = bw::Position(walkTile) + bw::Position{ 4, 4 };
    const bw::Position head = tail + vector * length;

    g_game->drawLineMap(tail, head, color);
    g_game->drawCircleMap(head, 1, bw::Colors::Orange, true);
}

void VectorField::drawPolyLine(std::vector<bw::WalkPosition> polyLine, bw::Color color) const {
    if (polyLine.size() < 2) {
        return;
    }

    for (auto& point : polyLine) {
        g_game->drawCircleMap(bw::Position(point), 3, bw::Colors::Red, true);
    }

    for (int i = 0; i < polyLine.size() - 1; i++) {
        const bw::Position start(polyLine.at(i));
        const bw::Position end(polyLine.at(i + 1));
        g_game->drawLineMap(start, end, bw::Colors::Red);
    }
}