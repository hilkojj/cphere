#include "level.h"
#include "../planet_generation/earth_generator.h"
#include "files/file.h"
#include "systems/buildings_system.h"
#include "systems/building_rendering_system.h"
#include "systems/ships_system.h"

Level::Level(const char *loadFilePath)
    :   earth("earth", Sphere(EARTH_RADIUS)),
        seaGraph(&earth),
        systems({

            new ShipsSystem(),
            new ShipPathSystem(),
            new BuildingsSystem(this),
            new BuildingRenderingSystem(this),
        })
{
    if (loadFilePath)
    {
        auto earthBin = File::readBinary(loadFilePath);
        earth.fromBinary(earthBin, [&]() {
            return earthMeshGenerator(&earth);
        });
    } else {
        generateEarth(&earth);
        std::vector<uint8> data;
        earth.toBinary(data);
        File::writeBinary("level.save", data);
    }

    for (Island *isl : earth.islands)
    {
        for (int i = 0; i < isl->width * isl->height * .8; i++)
        {
            int x = mu::randomInt(isl->width - 10), y = mu::randomInt(isl->height - 10);

            auto tree = Building(new Building_(&BLUEPRINTS::PINE_TREE, x, y, 0, isl));
            BLUEPRINTS::PINE_TREE.generator(tree);

            if (BuildingsSystem::active->canPlace(tree))
                BuildingsSystem::active->place(tree);
        }
    }

    seaGraph.generate();

    {   // tmp
        ships.push_back({ vec2(30, 30) });
    }
}

void Level::update(double deltaTime)
{
    time += deltaTime;

    for (auto sys : systems)
    {
        if (sys->disabled) continue;

        if (sys->updateFrequency == .0) sys->update(deltaTime, this);
        else
        {
            sys->updateAccumulator += deltaTime;
            while (sys->updateAccumulator > sys->updateFrequency)
            {
                sys->update(sys->updateFrequency, this);
                sys->updateAccumulator -= sys->updateFrequency;
            }
        }
    }
}

Level::~Level()
{
    for (auto sys : systems) delete sys;
}
