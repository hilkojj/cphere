
#ifndef SHIPS_H
#define SHIPS_H

#include "utils/math/interpolation.h"
#include "utils/math_utils.h"
#include "../level.h"
#include "input/key_input.h"
#include "input/mouse_input.h"

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

    float currentVelocity = 0., maxVelocity = 5.; // velocity in (distance-unit)/s

    float turning = 0.; // between 0-1, 0 = moving in straight line, 1 = making a turn
    int lastTurnDir = 0; // the direction the last turn, 1 -> clockwise, -1 -> counterclockwise
};

struct ShipPath
{
    std::vector<WayPoint> points;
    float progress = 0.;
};

class ShipsSystem : public LevelSystem
{
    static void initializeShip(Ship &ship, Level *lvl)
    {
        ship.pos = lvl->earth.lonLatTo3d(ship.lonLat.x, ship.lonLat.y, 0.);
        ship.goal = ship.pos;
        ship.dir = normalize(ship.pos - lvl->earth.lonLatTo3d(ship.lonLat.x + 10., ship.lonLat.y + 10., 0.));
        ship.up = normalize(ship.pos);

        ship.initialized = true;
    }

  public:
    void update(double deltaTime, Level *lvl) override
    {
        lvl->registry.view<Ship>().each([&](auto entity, Ship &ship) {

            if (!ship.initialized) initializeShip(ship, lvl);

            vec3 goalDir = normalize(ship.goal - ship.pos);
            vec3 newDir = ship.dir;

            // does the ship need to make a turn?
            int turn = 0;// 1 -> rotate clockwise, 0 -> dont turn, -1 -> rotate counterclockwise

            float distToGoal = length(ship.goal - ship.pos);
            
            if (distToGoal > .1) // ship is currently moving -> update the Direction of the ship
            {
                // this needs some visualization in order to understand this:
                float dirGoalDiff = length(goalDir - ship.dir);
                float rightGoalDiff = length(goalDir - ship.right);
                float leftGoalDiff = length(goalDir + ship.right);

                if (rightGoalDiff < dirGoalDiff && rightGoalDiff < leftGoalDiff)
                    turn = 1;
                else if (leftGoalDiff < dirGoalDiff)
                    turn = -1;

                if (turn != 0)
                {
                    if (ship.lastTurnDir != turn && ship.turning > .8)
                        turn = ship.lastTurnDir; // dont change turn direction mid-turn

                    ship.lastTurnDir = turn;
                    newDir = mix(ship.dir, ship.right * float(turn), deltaTime);
                    newDir = normalize(newDir);

                    ship.turning = mix<float>(ship.turning, 1., deltaTime * 1.5);
                } else {
                    ship.turning = mix<float>(ship.turning, 0, deltaTime);
                    float dirLerp = deltaTime * 2.;
                    newDir = mix(ship.dir, goalDir, deltaTime * 2.);
                    newDir = normalize(newDir);
                }
                ship.currentVelocity = ship.maxVelocity;
                ship.currentVelocity *= clamp(1. - ship.turning, .12, 1.);
                ship.currentVelocity *= clamp(distToGoal * .2, 0., 1.);
                ship.pos += newDir * ship.currentVelocity * float(deltaTime);
                ship.pos = normalize(ship.pos) * lvl->earth.sphere.radius;
            } else ship.currentVelocity = 0.;

            ship.dir = newDir;
            vec3 oldUp = ship.up;
            ship.up = normalize(ship.pos);

            ship.right = cross(ship.dir, ship.up);

            // tilt ship when making turn:
            ship.up = mix(
                oldUp,
                rotate(ship.up, ship.turning * mu::DEGREES_TO_RAD * float(-25. * ship.lastTurnDir), ship.dir),
                deltaTime * 2.
            );

            mat4 t = inverse(lookAt(ship.pos, ship.pos + ship.dir, ship.up));
            ship.transform = t;

            ship.lonLat = vec2(lvl->earth.longitude(ship.pos.x, ship.pos.z), lvl->earth.latitude(ship.pos.y));

            // tmp:
            if (MouseInput::justPressed(GLFW_MOUSE_BUTTON_RIGHT))
            {
                vec2 goalLonLat;
                if (lvl->earth.cursorToLonLat(lvl->cam, goalLonLat) && !lvl->earth.islAtLonLat(goalLonLat))
                {
                    std::vector<WayPoint> points;
                    lvl->seaGraph.findPath(ship.lonLat, goalLonLat, points);

                    lvl->registry.assign_or_replace<ShipPath>(entity, points);
                }
            }

            lvl->lineRenderer->line(ship.goal, ship.goal + ship.up * float(20.), mu::Z + mu::X);
            lvl->lineRenderer->line(ship.pos, ship.pos + ship.dir * float(20.), mu::Z);
            lvl->lineRenderer->line(ship.pos, ship.pos + ship.up * float(20.), mu::Y);
            lvl->lineRenderer->line(ship.pos, ship.pos + ship.right * float(20.), mu::X);
            // lvl->lineRenderer->line(ship.pos, ship.pos - normalize(eigelijkNewDir) * float(10.), mu::X + mu::Y);
        });
    }
};

class ShipPathSystem : public LevelSystem
{
  public:
    ShipPathSystem()
    {
        updateFrequency = 1./20.;
    }

    void update(double deltaTime, Level *lvl) override
    {
        lvl->registry.view<Ship, ShipPath>().each([&](auto entity, Ship &ship, ShipPath &path) {

            if (path.progress == 0.) path.progress = ship.currentVelocity / 10.; // maintain lead for goal on ship

            path.progress += (deltaTime * 2. * (1. - clamp(length(ship.goal - ship.pos) * .1, 0., 1.)));
            if (path.progress >= path.points.size() - 1)
            {
                lvl->registry.remove<ShipPath>(entity);
                return;
            }

            int wp0 = path.progress, wp1 = wp0 + 1;
            float lerp = path.progress - wp0;
            ship.goal = mix(path.points[wp0].position, path.points[wp1].position, lerp);
        });
    }
};

#endif
