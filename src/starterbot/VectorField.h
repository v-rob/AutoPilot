#pragma once

#include "Tools.h"
#include "Grid.h"
#include "UnitManager.h"
#include <optional>
#include <vector>

class Vector : public bw::Point<float, 1> {
public:
    // Constructors for Vector
    Vector(float x_, float y_);             // regular
    Vector(int x_, int y_);                 // casts int -> float
    Vector(const bw::Point<float, 1>& p);   // casts Point -> Vector
    Vector(const bw::Position& p);          // casts Position -> Vector
    Vector(double angle);                   // creates a normal vector with the given angle

    // returns the length of the vector in pixels
    float length();

    // changes the length of the vector to 1 but maintains the same angle
    void normalize();
};

namespace util {
    // keeps angle between 0 and 2 * PI
    double boundAngle(double angle);

    // returns the angle between two points
    double angleBetween(const Vector& point1, const Vector& point2);

    // returns the distance between two points
    float distanceBetween(const Vector& point1, const Vector& point2);
}

class VectorField : public EventReceiver {
private:
    UnitManager& m_unitManager;
    bw::Unitset m_aliveBuildings;

    // true (walkable) or false (not walkable) for every walk tile
    // char is being treated as boolean (C++ hates std::vector<bool>)
    Grid<char> m_walkable;

    // field of vectors pointing away from ground terrain / buildings
    Grid<std::optional<Vector>> m_groundField;  

    // field of vectors pointing away from enemies
    Grid<std::optional<Vector>> m_enemyField;

    // the radius (in walk tiles) of surrounding vectors to update for buildings
    const int BUILDING_MARGIN = 8;

    int m_width = 0;
    int m_height = 0;
    bool m_drawField = true;

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

    // Returns sum of vectors at a specific point
    std::optional<Vector> getVectorSum(int x, int y) const;

    void draw() const;
    void drawWalkTile(bw::WalkPosition walkTile, bw::Color color) const;
    void drawBuildTile(bw::TilePosition buildTile, bw::Color color) const;
    void drawWalkVector(bw::WalkPosition walkTile, Vector vector, bw::Color color) const;
    void drawBuildingTile(bw::Unit building);
    void drawFreeBuildingTile(bw::Unit building);
};