#pragma once

#include "Tools.h"
#include "Grid.h"
#include <vector>

class Vector : public bw::Point<float, 1> {
public:
    Vector(float x_, float y_);
};

class VectorField : public EventReceiver {
    Grid<bool> m_walkable;
    Grid<Vector> m_groundField;
    int m_width = 0;
    int m_height = 0;
    bool m_drawField = true;

    //bool canWalk(int walkX, int walkY) const;

public:

    VectorField();

    virtual void    onStart() override;
    virtual void    onFrame() override;

    void draw() const;
};