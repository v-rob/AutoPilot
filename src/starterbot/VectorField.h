#pragma once

#include "Tools.h"
#include "Grid.h"
#include "UnitManager.h"
#include <optional>
#include <vector>


class Vector2 : public bw::Point<float, 1> {
public:
    // Constructors for Vector2
    Vector2(float x_, float y_);             // regular
    Vector2(int x_, int y_);                 // casts int -> float
    Vector2(const bw::Point<float, 1>& p);   // casts Point -> Vector2
    Vector2(const bw::Position& p);          // casts Position -> Vector2
    Vector2(double angle);                   // creates a normal vector with the given angle

    // returns the length of the vector in pixels
    float length();

    // changes the length of the vector to 1 but maintains the same angle
    void normalize();
};

namespace util {
    // keeps angle between 0 and 2 * PI
    double boundAngle(double angle);

    // returns the angle between two points
    double angleBetween(const Vector2& point1, const Vector2& point2);

    // returns the distance between two points
    float distanceBetween(const Vector2& point1, const Vector2& point2);

    // returns the two points of intersection between two circles
    // c1, c2 = center of each circle
    // r1, r2 = radius of each circle
    std::pair<bw::Position, bw::Position> circleIntersection(const Vector2& c1, const Vector2& c2, int r1, int r2);

    // returns the convex hull of a set of points
    std::vector<bw::Position> convexHull(std::vector<bw::Position> points);
}

class VectorField : public EventReceiver {
private:
    UnitManager& m_unitManager;
    bw::Unitset m_aliveBuildings;

    // true (walkable) or false (not walkable) for every walk tile
    // char is being treated as boolean (C++ hates std::vector<bool>)
    Grid<char> m_walkable;

    // field of vectors pointing away from ground terrain / buildings
    Grid<std::optional<Vector2>> m_groundField;  

    // field of vectors pointing away from enemies
    Grid<std::optional<Vector2>> m_enemyField;

    // the radius (in walk tiles) of surrounding vectors to update for buildings
    const int BUILDING_MARGIN = 8;

    // a polyline defining the arc around an enemy base that surrounding vectors will point towards
    std::vector<bw::WalkPosition> m_path = { {100, 100}, {100, 150}, {150, 200} };

    int m_width = 0;
    int m_height = 0;
    bool m_drawField = true;

    // The chosen unit type for scouting (will eventually be dyanmic based on race / strategy)
    const bw::UnitType m_scoutType = bw::UnitTypes::Protoss_Scout;

    bw::Position m_mouse;
    std::vector<bw::WalkPosition> m_mouseTiles;

public:
    VectorField(UnitManager& unitManager);

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onSendText(const std::string& text) override;

    // Updates all of the vectors within a specified region
    void updateVectorRegion(bw::WalkPosition topLeft, bw::WalkPosition bottomRight, int margin);

    // Updates m_path based on the positions of enemy buildings
    void generatePath();

    // Returns sum of vectors at a specific point
    std::optional<Vector2> getVectorSum(int x, int y) const;

    void draw() const;
    void drawWalkTile(bw::WalkPosition walkTile, bw::Color color) const;
    void drawBuildTile(bw::TilePosition buildTile, bw::Color color) const;
    void drawWalkVector(bw::WalkPosition walkTile, Vector2 vector, bw::Color color) const;
    void drawPolyLine(std::vector<bw::WalkPosition> polyLine, bw::Color color) const;

    void drawBuildingTile(bw::Unit building);
    void drawFreeBuildingTile(bw::Unit building);
};