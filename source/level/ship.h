
#ifndef SHIP_H
#define SHIP_H

#include <vector>
#include "utils/math_utils.h"
#include "sea_graph.h"

struct ShipPath
{
    std::vector<WayPoint> points;
    float progress = 0.;
};

struct Ship
{
    vec2 lonLat;    // longitude and latitude of the ship
    mat4 transform; // transform of the ship. Rotation, (scale) and position.
    vec3
            up,     // the normal of the planet at the position of this ship, aka normalized(pos)
            dir,    // direction the ship is pointing towards
            right,  // cross of up and dir
            pos,    // position of the ship in 3d
            goal;   // where this ship is moving to

    bool initialized = false;

    float currentVelocity = 0., maxVelocity = 5.; // velocity in (distance-unit)/s

    float turning = 0.; // between 0-1, 0 = moving in straight line, 1 = making a turn
    int lastTurnDir = 0; // the direction the last turn, 1 -> clockwise, -1 -> counterclockwise

    std::optional<ShipPath> path;
};

#endif
