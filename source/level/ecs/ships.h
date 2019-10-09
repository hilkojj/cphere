
#ifndef SHIPS_H
#define SHIPS_H

#include "utils/math_utils.h"
#include "../level.h"

struct Ship
{
  public:
    vec2 lonLat;    // longitude and latitude of the ship
    mat4 transform; // transform of the ship. Rotation, (scale) and position.
    vec3 
        up,     // the normal of the planet at the position of this ship, aka normalized(pos)
        dir,    // direction the ship is pointing towards
        pos,    // position of the ship in 3d
        goal;   // where this ship is moving to
    bool initialized = false;
};

struct ShipPath
{
    std::vector<WayPoint> points;
    float progress = 0.;
};

class ShipsSystem : public LevelSystem
{
  public:
    void update(double deltaTime, Level *lvl)
    {
        lvl->entities.view<Ship>().each([&](auto entity, Ship &ship) {

            if (!ship.initialized)
            {
                ship.pos = lvl->earth.lonLatTo3d(ship.lonLat.x, ship.lonLat.y, 0.);
                ship.goal = ship.pos;
                ship.dir = normalize(ship.pos - lvl->earth.lonLatTo3d(ship.lonLat.x + 10., ship.lonLat.y + 10., 0.));

                ship.initialized = true;
            }

            vec3 prevPos = ship.pos;

            float goalLerp = float(min(1., deltaTime));
            ship.pos *= 1 - goalLerp;
            ship.pos += ship.goal * goalLerp;

            ship.lonLat = vec2(lvl->earth.longitude(ship.pos.x, ship.pos.z), lvl->earth.latitude(ship.pos.y));
            vec3 newDir = ship.pos - prevPos;
            if (length(newDir) <= deltaTime) newDir = ship.dir;
            newDir = normalize(newDir);
            
            ship.dir = newDir;
            ship.up = normalize(ship.pos);

            mat4 t = inverse(lookAt(ship.pos, ship.pos + ship.dir, ship.up));
            ship.transform = t;

            if (!lvl->entities.has<ShipPath>(entity) && mu::random() < deltaTime)
            {
                std::vector<WayPoint> points;
                lvl->seaGraph.findPath(ship.lonLat, lvl->seaGraph.nearest(vec2(mu::random(360.), mu::random(180.)))->lonLat, points);
                lvl->entities.assign<ShipPath>(entity, points);
            }
        });
    }
};

class ShipPathSystem : public LevelSystem
{
  public:
    void update(double deltaTime, Level *lvl)
    {
        lvl->entities.view<Ship, ShipPath>().each([&](auto entity, Ship &ship, ShipPath &path) {

            path.progress += deltaTime * .7;
            if (path.progress >= path.points.size() - 1)
            {
                lvl->entities.remove<ShipPath>(entity);
                return;
            }

            int wp0 = path.progress, wp1 = wp0 + 1;
            float lerp = path.progress - wp0;

            ship.goal = path.points[wp0].position * float(1. - lerp) + path.points[wp1].position * lerp;
        });
    }
};

#endif
