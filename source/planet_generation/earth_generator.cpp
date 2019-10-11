
#include "earth_generator.h"
#include "island_shape_generator.h"
#include "utils/math_utils.h"
#include "FastNoise.h"
#include "utils/math/interpolation.h"
#include "utils/json_model_loader.h"
#include "graphics/3d/tangent_calculator.h"
#include "utils/math/sphere_mesh_generator.h"

namespace
{

const float SEA_BOTTOM = -5, SEA_LEVEL = 0, LAND_LEVEL = 1.0f;
const int ROCK_TEX = 0, GRASS_TEX = 1, DEAD_GRASS_TEX = 2, ROCK2_TEX = 3;

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
    FastNoise hillNoise;
    for (int x = 0; x <= isl->width; x++)
    {
        for (int y = 0; y <= isl->height; y++)
        {
            float hilliness = Interpolation::pow(isl->distToHeight(x, y, SEA_BOTTOM, SEA_LEVEL + .1, 20) / 20., 2);
            float hillHeight = clamp(hillNoise.GetSimplexFractal(x * 1.6, y * 1.6) * .5 + .2, 0., 1.);
            isl->vertexPositionsOriginal[isl->xyToVertI(x, y)].y += hilliness * hillHeight * 4.;
        }
    }
    FastNoise mountainNoise(isl->width * isl->height);
    for (int x = 0; x <= isl->width; x++)
    {
        for (int y = 0; y <= isl->height; y++)
        {
            float mountainNess = isl->distToHeight(x, y, SEA_BOTTOM, SEA_LEVEL + .1, 5) / 5.;
            mountainNess *= 1. - isl->distToHeight(x, y, SEA_BOTTOM, SEA_LEVEL + .1, 20) / 20.;

            mountainNess *= clamp(mountainNoise.GetSimplexFractal(x, y) * 10., 0., 1.);
            mountainNess *= clamp(mountainNoise.GetSimplexFractal(x + 200., y + 200.) * 10., 0., 1.);

            float mountainHeight = clamp(mountainNoise.GetSimplexFractal(x * 2.2, y * 2.2) * .5 + .25, 0., 1.);

            isl->vertexPositionsOriginal[isl->xyToVertI(x, y)].y += mountainNess * mountainHeight * 23.;
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
            float height = isl->vertexPositionsOriginal[isl->xyToVertI(x, y)].y;
            if (height < 0 || height > 7) continue;

            float rockness = clamp(isl->tileSteepness(x, y) - .01, 0., 1.);

            int maxDist = 1 + (int)(30 * abs(noise.GetNoise(x * 6, y * 6)));
            float dist = max(0.0f, isl->distToHeight(x, y, -100, 0, maxDist + 3) - 3);
            isl->textureMap[isl->xyToVertI(x, y)][GRASS_TEX] = clamp<float>(Interpolation::powOut(dist / maxDist, 2) - rockness, 0., 1.);
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
            if (isl->textureMap[isl->xyToVertI(x, y)][GRASS_TEX] != 1) continue;

            float noiseX = x + noiseOffset, noiseY = y + noiseOffset;

            noiseX += noise.GetNoise(noiseX, noiseY) * 100;
            noiseY += noise.GetNoise(noiseY, noiseX) * 100;

            isl->textureMap[isl->xyToVertI(x, y)][DEAD_GRASS_TEX] = clamp((noise.GetNoise(noiseX, noiseY) - .2) * 3. + isl->textureMap[isl->xyToVertI(x, y)][DEAD_GRASS_TEX], 0.0, 1.0);
        }
    }
}

void addRockTexture(Island *isl)
{
    for (int x = 0; x <= isl->width; x++)
    {
        for (int y = 0; y <= isl->height; y++)
        {
            float height = isl->vertexPositionsOriginal[isl->xyToVertI(x, y)].y;
            float rockness = clamp<float>(isl->tileSteepness(x, y), 0., 1.) + (height > 3 ? 1 : 0);
            rockness *= 1. - isl->distToHeight(x, y, 1.5, 999, 3) / 3.;
            isl->textureMap[isl->xyToVertI(x, y)][ROCK_TEX] = rockness;
        }
    }
}

void addRock2Texture(Island *isl)
{
    for (int x = 0; x <= isl->width; x++)
    {
        for (int y = 0; y <= isl->height; y++)
        {
            float height = isl->vertexPositionsOriginal[isl->xyToVertI(x, y)].y + isl->tileSteepness(x, y);

            if (height < 9.) continue;

            float maxHeight = 0.;
            mu::spiral(4, [&](ivec2 p) {
                if (p.x < 0 || p.y < 0 || p.x > isl->width || p.y > isl->height) return true;

                maxHeight = max(maxHeight, isl->vertexPositionsOriginal[isl->xyToVertI(p.x, p.y)].y);
                return true;
            });

            float snow = 1. - clamp<float>(maxHeight - height, 0., 1.);
            isl->textureMap[isl->xyToVertI(x, y)][ROCK2_TEX] = snow;
        }
    }
}

void islandTextureMapper(Island *isl)
{
    addGrass(isl);
    addDeadGrass(isl);
    addDeadGrass(isl);
    addRockTexture(isl);
    addRock2Texture(isl);
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
        [&](int islandNumber) {
            bool h = mu::random() > .5;
            return IslandContext{
                IslandGenerator(
                    mu::randomInt(100, h ? 150 : 250), mu::randomInt(100, h ? 250 : 150),
                    earth,
                    generateIslandTerrain,
                    islandTextureMapper),
                0, 180};
        },

        // nr of islands:
        14,

        // Earth mesh generator:
        [&]() {
            return earthMeshGenerator(earth);
        });

    g.generate();
}
