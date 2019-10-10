#ifndef LEVEL_H
#define LEVEL_H

#include "planet.h"
#include "sea_graph.h"
#include "../entt.hpp"
#include "graphics/3d/debug_line_renderer.h"

const float EARTH_RADIUS = 150, ATMOSPHERE_RADIUS = 198;

class Level;

class LevelSystem
{
  protected:
    float updateFrequency = 0; // update this system every n seconds. if n = 0 then update(deltaTime) is called, else update(n)
    float updateAccumulator = 0;

    bool disabled = false;

    virtual void update(double deltaTime, Level *lvl) = 0;

    friend Level;
};

class Level
{
  public:

    Level(const char *loadFilePath=NULL);

    Planet earth;
    SeaGraph seaGraph;
    entt::registry entities;

    DebugLineRenderer *lineRenderer;

    std::vector<LevelSystem*> systems;

    float time = 0;

    void update(double deltaTime);

    ~Level();
};

#endif
