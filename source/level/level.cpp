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
