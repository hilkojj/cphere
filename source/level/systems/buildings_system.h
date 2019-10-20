#ifndef BUILDINGS_SYSTEM_H
#define BUILDINGS_SYSTEM_H

#include <string>
#include <memory>
#include <map>
#include <functional>
#include <input/key_input.h>
#include "utils/gu_error.h"
#include "utils/math_utils.h"
#include "graphics/3d/mesh.h"
#include "input/mouse_input.h"
#include "../level.h"

class BuildingsSystem : public LevelSystem
{
  public:

    inline static BuildingsSystem *active = NULL;

    std::vector<std::function<void(Building)>> onPlaced, onDestroyed;

    Level *lvl;

    Building currentlyPlacing;

    BuildingsSystem(Level *lvl) : lvl(lvl)
    {
        active = this;
    }

    void update(double deltaTime, Level *lvl) override
    {
        int i = 0;

        for (Blueprint *bp : BLUEPRINTS::list)
        {
            if (KeyInput::justPressed(GLFW_KEY_1 + i++))
            {
                std::cout << "Start placing of " << bp->name << "\n";
                currentlyPlacing = Building(new Building_(bp, 0, 0, 0, NULL));
                bp->generator(currentlyPlacing);
            }
        }
        if (KeyInput::justPressed(GLFW_KEY_DELETE))
        {
            auto *isl = lvl->earth.islUnderCursor(lvl->cam);
            if (isl)
            {
                ivec2 tile;
                if (isl->tileUnderCursor(tile, lvl->cam))
                {
                    auto b = isl->getBuilding(tile.x, tile.y);
                    if (b) destroy(b);
                }
            }
        }
        if (KeyInput::justPressed(GLFW_KEY_ESCAPE))
            currentlyPlacing = NULL;

        if (currentlyPlacing)
        {
            bool blocked = true;
            auto *isl = lvl->earth.islUnderCursor(lvl->cam);
            if (isl)
            {
                ivec2 tile;
                if (isl->tileUnderCursor(tile, lvl->cam))
                {
                    currentlyPlacing->move(
                            tile.x,
                            tile.y,
                            currentlyPlacing->rotation + (MouseInput::justPressed(GLFW_MOUSE_BUTTON_MIDDLE) ? 1 : 0),
                            isl
                    );
                    blocked = !canPlace(currentlyPlacing);
                }
            }
            if (MouseInput::justPressed(GLFW_MOUSE_BUTTON_LEFT))
            {
                if (blocked)
                    std::cout << "BUILDING BLOCKED!" << "\n";
                else {
                    place(currentlyPlacing);
                    // stop placing building
                    currentlyPlacing = NULL;
                }
            }
        }
    }

    bool canPlace(Building b)
    {
        for (auto &t : b->tiles)
        {
            if (!b->isl->containsTile(t.x, t.y)) return false;
            if (!b->isl->tileAboveSea(t.x, t.y)) return false;
            if (b->isl->getBuilding(t.x, t.y)) return false;
            if (b->bp->needsFlatGround && b->isl->tileSteepness(t.x, t.y) > 1) return false;
        }
        return true;
    }

    void place(Building b)
    {
        std::cout << "PLACING " << b->bp->name << "\n";

        b->isl->buildingsPerBlueprint[b->bp].push_back(b);

        for (auto &p : b->tiles)
        {
            if (b->isl->buildingsGrid[p.x][p.y])
                throw gu_err("ATTEMPT TO PLACE BUILDING ON ALREADY OCCUPIED TILE");

            b->isl->buildingsGrid[p.x][p.y] = b;
        }
        for (auto &f : onPlaced) f(b);
    }

    void destroy(Building b)
    {
        std::cout << b->bp->name << " DESTROYED at " << to_string(b->tiles[0]) << " on Island " << b->isl << "\n";

        auto &vec = b->isl->buildingsPerBlueprint[b->bp];
        vec.erase(std::remove(vec.begin(), vec.end(), b), vec.end());

        for (auto &p : b->tiles)
        {
            if (b->isl->buildingsGrid[p.x][p.y] != b)
                throw gu_err("ATTEMPT TO REMOVE BUILDING THAT WAS NOT PLACED ON SAID TILES");

            b->isl->buildingsGrid[p.x][p.y] = 0;
        }
        for (auto &f : onDestroyed) f(b);
    }

};

#endif
