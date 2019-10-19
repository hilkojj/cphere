#ifndef LEVEL_H
#define LEVEL_H

#include <map>

#include "planet.h"
#include "sea_graph.h"
#include "graphics/3d/debug_line_renderer.h"
#include "ship.h"

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

    std::vector<LevelSystem*> systems;
    std::vector<Ship> ships;

    float time = 0;

    void update(double deltaTime);

    // consider removing the following variables because they're graphics related:
    DebugLineRenderer *lineRenderer;
    Camera *cam;

    ~Level();
};

#endif
