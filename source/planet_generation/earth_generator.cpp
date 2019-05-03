
#include "earth_generator.h"
#include "island_shape_generator.h"
#include "utils/math_utils.h"

namespace
{

static float SEA_BOTTOM = -5, SEA_LEVEL = 0, LAND_LEVEL = .7f;

void terrainFromShape(std::vector<bool> shape, Island *isl)
{
    for (int x = 0; x <= isl->width; x++)
    {
        for (int y = 0; y <= isl->height; y++)
        {
            int beachWidth = 10;// + (int)(14 * glm::abs(SimplexNoise.noise(x * .02f, y * .02f)));
            float distToSea = beachWidth;

            for (int x0 = glm::max(0, x - beachWidth); x0 <= glm::min(isl->width, x + beachWidth); x0++)
            {
                for (int y0 = glm::max(0, y - beachWidth); y0 <= glm::min(isl->height, y + beachWidth); y0++)
                {
                    if (!shape[isl->xyToVertI(x0, y0)])
                    {
                        // is sea.
                        int xDiff = x - x0;
                        int yDiff = y - y0;
                        distToSea = glm::min(distToSea, (float)glm::sqrt(xDiff * xDiff + yDiff * yDiff));
                    }
                }
            }
            float height = distToSea / beachWidth; //Interpolation.circleOut.apply(distToSea / beachWidth);
            height = SEA_BOTTOM + height * glm::abs(SEA_BOTTOM - LAND_LEVEL);
            isl->vertexPositionsOriginal[isl->xyToVertI(x, y)].y = height;
        }
    }
}

void generateIslandTerrain(Island *isl)
{
    isl->seaBottom = SEA_BOTTOM;
    terrainFromShape(IslandShapeGenerator(isl).shape, isl);
}

void islandTextureMapper(Island *isl)
{
}

} // namespace

void generateEarth(Planet *earth)
{
    PlanetGenerator g(
        earth,

        // Island context provider:
        [&]() {
            return IslandContext{
                IslandGenerator(
                    mu::randomInt(95, 130), mu::randomInt(95, 130),
                    earth,
                    generateIslandTerrain,
                    islandTextureMapper),
                0, 180};
        },

        // nr of islands:
        5,

        // Earth mesh generator:
        [&]() {
            return SharedMesh(new Mesh(earth->name + "_mesh", 0, 0, VertAttributes()));
        });

    g.generate();
}
