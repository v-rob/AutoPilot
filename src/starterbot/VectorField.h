#pragma once

#include "Tools.h"
#include "Grid.h"
#include "UnitManager.h"
#include <optional>
#include <vector>

// TODO: The current implementation of VectorField assumes only one enemy base, so the path
// generation falters once they expand. m_enemyBuildings should eventually be partioned into
// different sets (one for each base) and there should be a seperate path around each


class Vector2 : public bw::Point<float, 1> {
public:
    // Constructors for Vector2
    Vector2(float x_, float y_);             // regular
    Vector2(int x_, int y_);                 // casts int -> float
    Vector2(double x_, double y_);           // casts double -> float
    Vector2(const bw::Point<float, 1>& p);   // casts Point -> Vector2
    Vector2(const bw::Position& p);          // casts Position -> Vector2
    Vector2(double angle);                   // creates a normal vector with the given angle

    // Returns the length of the vector in pixels
    float length();

    // Changes the length of the vector to 1 but maintains the same angle
    Vector2 normalize();

    // Rotate the vector by a given angle
    Vector2 rotate(double angle);
};

namespace util {
    // Keeps angle between 0 and 2 * PI
    double boundAngle(double angle);

    // Returns the angle between two points
    template <typename T>
    double angleBetween(const T& point1, const T& point2);

    // Returns the distance between two points
    float distanceBetween(const Vector2& point1, const Vector2& point2);

    // Returns the convex hull of a set of points
    // The returned hull starts with the left-most point and the following ones are in clockwise order
    std::vector<bw::Position> convexHull(std::vector<bw::Position> points);

    // Given a point p and a line segment l, returns the point p' on l that is closest to p
    bw::Position closestPointOnSegment(bw::Position point, std::pair<bw::Position, bw::Position> segment);
}

class VectorField : public EventReceiver {
private:
    // scale factor for WalkPosition -> TilePosition (32 / 8 = 4)
    const int WALKPOS_PER_TILEPOS = bw::TILEPOSITION_SCALE / bw::WALKPOSITION_SCALE;

    // Assumed max base width in walk tiles
    const int BASE_WIDTH = 100;

    // The radius (in walk tiles) of surrounding vectors to update for buildings
    const int BUILDING_MARGIN = 8;

    UnitManager& m_unitManager;
    bw::Unitset m_enemyBuildings;

    // Maps known base locations -> players. This should eventually move to a higher class
    // since it will be helpful to StrategyManager
    //std::unordered_map<bw::TilePosition, bw::Player> m_baseLocations;

    // A polyline defining the arc around an enemy base that surrounding vectors will point towards
    std::vector<bw::WalkPosition> m_path;

    // Field of vectors pointing towards m_path
    Grid<std::optional<Vector2>> m_pathField;

    // Field of vectors pointing away from ground terrain / buildings
    Grid<std::optional<Vector2>> m_groundField;  

    // Field of vectors pointing away from enemies
    Grid<std::optional<Vector2>> m_enemyField;

    // true (walkable) or false (not walkable) for every walk tile
    // char is being treated as boolean (C++ hates std::vector<bool>)
    Grid<char> m_walkable;

    // width and height of the map in walk tiles
    int m_width = 0;
    int m_height = 0;

    bool m_drawField = false;

    // The chosen unit type for scouting (will eventually be dyanmic based on race / strategy)
    const bw::UnitType m_scoutType = bw::UnitTypes::Protoss_Scout;

    bw::Position m_mouse;

public:
    VectorField(UnitManager& unitManager);
    // Returns sum of vectors at a specific point
    std::optional<Vector2> getVectorSum(int x, int y) const;

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onSendText(const std::string& text) override;

    // Updates m_path based on the positions of enemy buildings
    void generatePath();

    // Updates the vectors in m_pathField to point towards m_path
    void updatePathField();

    // Updates the vectors in m_groundField to point away from a building/resource
    void updateGroundField(bw::Unit unit, int margin);

    // Updates the vectors in m_enemyField to point away from a given enemy troop's position
    void updateEnemyField(bw::WalkPosition position);

    // Update the walkable values under a given building/resource
    void updateWalkable(bw::Unit unit, bool value);


    void draw() const;
    void drawWalkTile(bw::WalkPosition walkTile, bw::Color color) const;
    void drawBuildTile(bw::TilePosition buildTile, bw::Color color) const;
    void drawWalkVector(bw::WalkPosition walkTile, Vector2 vector, bw::Color color) const;
    void drawPolyLine(std::vector<bw::WalkPosition> polyLine, bw::Color color) const;
};