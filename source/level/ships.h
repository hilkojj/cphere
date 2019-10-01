
#pragma once

#include "utils/math_utils.h"
#include "sea_graph.h"

class Ship
{
  public:
    vec2 lonLat;
    vec3 position;

    std::vector<WayPoint> path;
    float pathProgress;
};  

class ShipManager
{
  public:
    std::vector<Ship> ships;

    void update(double deltaTime);
};
