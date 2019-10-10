
#ifndef SHIPS_H
#define SHIPS_H

#include "utils/math_utils.h"
#include "../level.h"
#include "input/key_input.h"

struct Ship
{
  public:
    vec2 lonLat;    // longitude and latitude of the ship
    mat4 transform; // transform of the ship. Rotation, (scale) and position.
    vec3 
        up,     // the normal of the planet at the position of this ship, aka normalized(pos)
        dir,    // direction the ship is pointing towards
        right,  // cross of up and dir
        pos,    // position of the ship in 3d
        goal;   // where this ship is moving to
    bool initialized = false;
    float changingDir = 0.; // between 0-1, 0 = moving in straight line, 1 = making a turn
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
                ship.up = normalize(ship.pos);

                ship.initialized = true;
            }

            vec3 prevPos = ship.pos;

            float goalLerp = float(min(1., deltaTime));// * (1. - .8 * ship.changingDir);
            ship.pos *= 1 - goalLerp;
            ship.pos += ship.goal * goalLerp;

            ship.lonLat = vec2(lvl->earth.longitude(ship.pos.x, ship.pos.z), lvl->earth.latitude(ship.pos.y));
            vec3 newDir = normalize(ship.pos - prevPos);
            vec3 eigelijkNewDir = newDir;
            if (length(prevPos - ship.pos) <= deltaTime) // ship doesn't move
                newDir = ship.dir;
            else if (length(newDir - ship.dir) > length(newDir - ship.right))
            {
                std::cout << "draaaaaaaaaaaaaai rechts\n";

                float lerp = deltaTime;  
                newDir = ship.dir * (1 - lerp) + ship.right * lerp;
                newDir = normalize(newDir);
                ship.changingDir = 1.;

            } else if (length(newDir - ship.dir) > length(newDir + ship.right)) {
                std::cout << "draaaaaaaaaaaaaai links\n";

                float lerp = deltaTime;  
                newDir = ship.dir * (1 - lerp) - ship.right * lerp;
                newDir = normalize(newDir);

                ship.changingDir = 1.;

            } else {
                
                ship.changingDir -= deltaTime;
                float dirLerp = deltaTime * 2.;
                newDir = normalize(newDir * dirLerp + ship.dir * float(1. - dirLerp));
            }

            


            ship.dir = newDir;
            ship.up = normalize(ship.pos);
            ship.right = cross(ship.dir, ship.up);

            mat4 t = inverse(lookAt(ship.pos, ship.pos + ship.dir, ship.up));
            ship.transform = t;

            if (!lvl->entities.has<ShipPath>(entity) && mu::random() < deltaTime)
            {
                std::vector<WayPoint> points;
                lvl->seaGraph.findPath(ship.lonLat, lvl->seaGraph.nearest(vec2(mu::random(360.), mu::random(180.)))->lonLat, points);
                lvl->entities.assign<ShipPath>(entity, points);
            }

            lvl->lineRenderer->line(ship.pos, ship.pos + ship.dir * float(20.), mu::Z);
            lvl->lineRenderer->line(ship.pos, ship.pos + ship.up * float(20.), mu::Y);
            lvl->lineRenderer->line(ship.pos, ship.pos + ship.right * float(20.), mu::X);
            lvl->lineRenderer->line(ship.pos, ship.pos - normalize(eigelijkNewDir) * float(10.), mu::X + mu::Y);
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
