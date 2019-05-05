
#include "earth_generator.h"
#include "island_shape_generator.h"
#include "utils/math_utils.h"
#include "FastNoise.h"
#include "utils/math/interpolation.h"
#include "utils/json_model_loader.h"
#include "graphics/3d/tangent_calculator.h"

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
            int beachWidth = 10 + (int)(20 * glm::abs(noise.GetNoise(x * 3, y * 3)));
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
            float height = Interpolation::powOut(distToSea / beachWidth, 5);
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

SharedMesh earthMeshGenerator(Planet *earth)
{
    VertAttributes attrs;
    attrs.add(VertAttributes::POSITION);
    attrs.add(VertAttributes::NORMAL);
    attrs.add(VertAttributes::TANGENT);
    int texOffset = attrs.add(VertAttributes::TEX_COORDS);
    auto loaded = JsonModelLoader::fromUbjsonFile("assets/models/earth_sphere.ubj", &attrs);
    assert(loaded.size() == 1 && "Expected 1 loaded earth model");
    assert(loaded[0]->parts.size() == 1 && "Expected 1 earth model part");
    SharedMesh mesh = loaded[0]->parts[0].mesh;
    float radius = earth->sphere.radius;
    for (int i = 0; i < mesh->nrOfVertices * attrs.getVertSize(); i += attrs.getVertSize())
    {
        auto &x = mesh->vertices[i],
             &y = mesh->vertices[i + 1],
             &z = mesh->vertices[i + 2];
        x *= radius;
        y *= radius;
        z *= radius;

        mesh->vertices[i + texOffset] = earth->longitude(x, z) / 360;
        mesh->vertices[i + texOffset + 1] = earth->latitude(y) / 180;
    }
    TangentCalculator::addTangentsToMesh(mesh.get());
    return mesh;
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
                    mu::randomInt(100, 150), mu::randomInt(100, 150),
                    earth,
                    generateIslandTerrain,
                    islandTextureMapper),
                0, 180};
        },

        // nr of islands:
        5,

        // Earth mesh generator:
        [&]() {
            return earthMeshGenerator(earth);
        });

    g.generate();
}
