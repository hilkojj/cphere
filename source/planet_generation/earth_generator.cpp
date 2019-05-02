
#include "earth_generator.h"


namespace
{

void generateIslandTerrain(Island *isl)
{
    for (auto &p : isl->vertexPositionsOriginal) p.y = -10;
    isl->vertexPositionsOriginal[isl->width * 3 + 10].y = 30;
    isl->vertexPositionsOriginal[isl->width * 3 + 11].y = 30;
}

void islandTextureMapper(Island *isl)
{
}

}

void generateEarth(Planet *earth)
{
    PlanetGenerator g(
        earth,

        // Island context provider:
        [&]() {
            return IslandContext{
                IslandGenerator(
                    50, 50,
                    earth,
                    generateIslandTerrain, 
                    islandTextureMapper
                ),
                0, 180
            };
        },

        // nr of islands:
        10,

        // Earth mesh generator:
        [&]() {
            return SharedMesh(new Mesh(earth->name + "_mesh", 0, 0, VertAttributes()));
        });

    g.generate();
}
