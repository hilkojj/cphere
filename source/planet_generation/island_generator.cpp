#include "island_generator.h"
#include "island_outliner.h"
#include "utils/math_utils.h"
#include "../level/planet.h"
#include "glm/gtx/rotate_vector.hpp"
#include "graphics/3d/model_instance.h"

static const float ISLAND_ROTATION = 30;

IslandGenerator::IslandGenerator(int width, int height, Planet *plt, TerrainGenerator terrainGenerator, TextureMapper textureMapper)
    : terrainGenerator(terrainGenerator), textureMapper(textureMapper)
{
    reset(width, height, plt);
}

Island *IslandGenerator::generateEssentials()
{
    if (generated)
    {
        std::cerr << "WARNING: same IslandGenerator used twice";
        return isl;
    }
    while (!tryToGenerate()) reset(isl->width, isl->height, isl->planet);
    generated = true;
    return isl;
}

void IslandGenerator::finishGeneration()
{
    isl->calculateNormals();
    textureMapper(isl);
    isl->createMesh();
}

void IslandGenerator::reset(int width, int height, Planet *plt)
{
    if (isl) delete isl;
    isl = new Island(width, height, plt);
}

bool IslandGenerator::tryToGenerate()
{
    initVertexPositions();
    terrainGenerator(isl);

    IslandOutliner outliner(isl, 0);
    outliner.getOutlines(isl->outlines2d);

    if (isl->outlines2d.size() == 0) return false;
    
    // outlines are incorrect in rare cases, so check them and return false if outlines are incorrect.
    if (!outliner.checkOutlines(isl->outlines2d)) return false;

    isl->planetDeform();
    return true;
}

void IslandGenerator::initVertexPositions()
{
    for (int i = 0; i < isl->nrOfVerts; i++)
        isl->vertexPositionsOriginal[i] = rotate(
            vec3(
                isl->vertIToX(i) - isl->width / 2,
                0,
                isl->vertIToY(i) - isl->width / 2
            ),
            ISLAND_ROTATION * mu::DEGREES_TO_RAD,
            mu::Y
        );
}
