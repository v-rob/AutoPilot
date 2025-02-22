#pragma once

#include "Tools.h"
#include "Grid.h"
#include "UnitManager.h"
#include <vector>

class Vector : public bw::Point<float, 1> {
public:
    Vector(float x_, float y_);
    Vector(const bw::Point<float, 1>& p);
    Vector(const bw::Position& p);
    Vector(double angle);

    float length();
    void normalize();
};

namespace util {
    // keeps angle between 0 and 2 * PI
    double boundAngle(double angle);

    // returns the angle between two points
    double angleBetween(const bw::Position& point1, const bw::Position& point2);

    // returns the distance between two points
    float distanceBetween(const Vector& point1, const Vector& point2);
}

class VectorField : public EventReceiver {
private:
    UnitManager& m_unitManager;
    bw::Unitset m_aliveBuildings;

    Grid<bool> m_walkable;
    Grid<Vector> m_groundField;
    int m_width = 0;
    int m_height = 0;
    bool m_drawField = true;

    bw::Position m_mouse;
    std::vector<bw::WalkPosition> m_mouseTiles;

    //bool canWalk(int walkX, int walkY) const;

public:
    VectorField(UnitManager& unitManager);

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onSendText(const std::string& text) override;

    void draw() const;
    void drawWalkTile(bw::WalkPosition walkTile, bw::Color color) const;
    void drawBuildTile(bw::TilePosition buildTile, bw::Color color) const;
    void drawWalkVector(bw::WalkPosition walkTile, Vector vector, bw::Color color) const;
    void drawBuildingTile(bw::Unit building);
    void drawFreeBuildingTile(bw::Unit building);
};