
#include "earth_generator.h"
#include "island_shape_generator.h"
#include "utils/math_utils.h"
#include "FastNoise.h"
#include "utils/math/interpolation.h"
#include "utils/json_model_loader.h"
#include "graphics/3d/tangent_calculator.h"
#include "utils/math/sphere_mesh_generator.h"
using namespace glm;

namespace
{

static float SEA_BOTTOM = -5, SEA_LEVEL = 0, LAND_LEVEL = 1.0f;

void terrainFromShape(std::vector<bool> shape, Island *isl)
{
    FastNoise noise;
    for (int x = 0; x <= isl->width; x++)
    {
        for (int y = 0; y <= isl->height; y++)
        {
            int beachWidth = 20 + (int)(20 * abs(noise.GetNoise(x / 2, y / 2)));
            float distToSea = beachWidth;

            for (int x0 = max(0, x - beachWidth); x0 <= min(isl->width, x + beachWidth); x0++)
            {
                for (int y0 = max(0, y - beachWidth); y0 <= min(isl->height, y + beachWidth); y0++)
                {
                    if (!shape[isl->xyToVertI(x0, y0)])
                    {
                        // is sea.
                        int xDiff = x - x0;
                        int yDiff = y - y0;
                        distToSea = min(distToSea, (float)sqrt(xDiff * xDiff + yDiff * yDiff));
                    }
                }
            }
            float height = Interpolation::powOut(distToSea / beachWidth, 2);
            height = SEA_BOTTOM + height * abs(SEA_BOTTOM - LAND_LEVEL);
            isl->vertexPositionsOriginal[isl->xyToVertI(x, y)].y = height;
        }
    }
}

void generateIslandTerrain(Island *isl)
{
    isl->seaBottom = SEA_BOTTOM;
    terrainFromShape(IslandShapeGenerator(isl).shape, isl);
}

void addGrass(Island *isl)
{
    FastNoise noise;
    for (int x = 0; x <= isl->width; x++)
    {
        for (int y = 0; y <= isl->height; y++)
        {
            if (isl->vertexPositionsOriginal[isl->xyToVertI(x, y)].y < 0) continue;

            int maxDist = 1 + (int)(10 * abs(noise.GetNoise(x * 3, y * 3)));
            float dist = max(0.0f, isl->distToHeight(x, y, -100, 0, maxDist + 3) - 3);
            isl->textureMap[isl->xyToVertI(x, y)][0] = Interpolation::powOut(dist / maxDist, 2);
        }
    }
}

void addDeadGrass(Island *isl)
{
    FastNoise noise;
    noise.SetNoiseType(FastNoise::SimplexFractal);

    float noiseOffset = mu::random(1000);

    for (int x = 0; x <= isl->width; x++)
    {
        for (int y = 0; y <= isl->height; y++)
        {
            if (isl->textureMap[isl->xyToVertI(x, y)][0] != 1) continue;

            float noiseX = x + noiseOffset, noiseY = y + noiseOffset;

            noiseX += noise.GetNoise(noiseX, noiseY) * 100;
            noiseY += noise.GetNoise(noiseY, noiseX) * 100;

            isl->textureMap[isl->xyToVertI(x, y)][1] = min(max((noise.GetNoise(noiseX, noiseY) - .2) * 3., 0.0), 1.0);
        }
    }
}

void islandTextureMapper(Island *isl)
{
    addGrass(isl);
    addDeadGrass(isl);
    addDeadGrass(isl);
}

} // namespace

SharedMesh earthMeshGenerator(Planet *earth)
{
    VertAttributes attrs;
    attrs.add_(VertAttributes::POSITION)
        .add_(VertAttributes::NORMAL)
        .add_(VertAttributes::TANGENT)
        .add_(VertAttributes::TEX_COORDS);
    return SphereMeshGenerator::generate(earth->name + "_mesh", earth->sphere.radius, 100, 70, attrs);
}

void generateEarth(Planet *earth)
{
    PlanetGenerator g(
        earth,

        // Island context provider:
        [&]() {
            return IslandContext{
                IslandGenerator(
                    mu::randomInt(100, 150), mu::randomInt(100, 150),
                    earth,
                    generateIslandTerrain,
                    islandTextureMapper),
                0, 180};
        },

        // nr of islands:
        1,

        // Earth mesh generator:
        [&]() {
            return earthMeshGenerator(earth);
        });

    g.generate();
}
