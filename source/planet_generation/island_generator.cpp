#include "island_generator.h"
#include "island_outliner.h"
#include "utils/math_utils.h"
#include "../level/planet.h"
#include "glm/gtx/rotate_vector.hpp"
#include "graphics/3d/model_instance.h"
#include "graphics/3d/tangent_calculator.h"

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
    calculateNormals();
    textureMapper(isl);
    createMesh();
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
    
    // outlines are incorrect in rare cases, so check them and return false if outlines are incorrect.
    if (!outliner.checkOutlines(isl->outlines2d)) return false;

    planetDeform();
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

void IslandGenerator::planetDeform()
{
    float radius = isl->planet->sphere.radius;
    vec3 planetOrigin = vec3(0, -radius, 0);

    // STEP 1: replace vertex positions
    for (int i = 0; i < isl->nrOfVerts; i++)
    {
        vec3 &original = isl->vertexPositionsOriginal[i];
        vec3 &deformed = isl->vertexPositions[i] = normalize(original - planetOrigin);
        deformed *= radius + original.y;
        deformed += planetOrigin;
    }
    // STEP 2: create outlines in 3d.
    for (Polygon &outline : isl->outlines2d)
    {
        isl->outlines3d.push_back(std::vector<vec3>());
        auto &outline3d = isl->outlines3d.back();
        outline3d.reserve(outline.points.size());
        int i = 0;
        for (vec2 &p2D : outline.points)
        {
            vec3 p3D = normalize(vec3(p2D.x, 0, p2D.y) - planetOrigin);
            p3D *= radius;
            p3D += planetOrigin;
            outline3d.push_back(p3D);
        }
    }
}

static int 
    lol1[]{0, 1, 0, -1},
    lol2[]{1, 0, -1, 0},
    lol3[]{0, -1, 0, 1};

void IslandGenerator::calculateNormals()
{
    for (int vertI = 0; vertI < isl->nrOfVerts; vertI++)
    {
        int x = isl->vertIToX(vertI), y = isl->vertIToY(vertI);
        vec3 normal = vec3();
        vec3 &p0 = isl->vertexPositions[vertI];
        for (int i = 0; i < 4; i++)
        {
            int x1 = x + lol1[i];
            int y1 = y + lol2[i];
            int x2 = x + lol2[i];
            int y2 = y + lol3[i];

            if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0 || x1 > isl->width || x2 > isl->width || y1 > isl->height || y2 > isl->height)
                continue;

            vec3 
                &p1 = isl->vertexPositions[isl->xyToVertI(x1, y1)],
                &p2 = isl->vertexPositions[isl->xyToVertI(x2, y2)];

            normal += mu::calculateNormal(p0, p1, p2);
        }
        isl->vertexNormals[vertI] = normalize(normal);
        isl->vertexNormalsPlanet[vertI] = isl->planetTransform * vec4(isl->vertexNormals[vertI], 0);
    }
}

void IslandGenerator::createMesh()
{
    std::string name = isl->planet->name + "_island_" + std::to_string(isl->planet->islands.size());
    VertAttributes attrs = VertAttributes();
    unsigned int
        posOffset = attrs.add(VertAttributes::POSITION),
        norOffset = attrs.add(VertAttributes::NORMAL),
        uvOffset = attrs.add(VertAttributes::TEX_COORDS),
        tanOffset = attrs.add(VertAttributes::TANGENT),
        texOffset = attrs.add({"TEX_BLEND", 4, GL_FALSE});

    SharedMesh mesh = SharedMesh(new Mesh(name + "_mesh", isl->nrOfVerts, isl->width * isl->height * 6, attrs));

    for (int i = 0; i < isl->nrOfVerts; i++)
    {
        mesh->setVec3(isl->vertexPositionsPlanet[i], i, posOffset);     // position
        mesh->setVec3(isl->vertexNormalsPlanet[i], i, norOffset);       // normal

        mesh->setVec2(
            vec2(isl->vertIToX(i), isl->vertIToY(i)) / vec2(50), i, uvOffset    // texCoords
        );

        mesh->setVec4(isl->textureMap[i], i, texOffset);    // texture blending
    }

    int i = 0;
    for (int y = 0; y < isl->height; y++)
    {
        for (int x = 0; x < isl->width; x++)
        {
            if (isl->tileAtSeaFloor(x, y)) continue;

            // triangle 1
            mesh->indices[i + 0] = isl->xyToVertI(x + 1, y + 1);
            mesh->indices[i + 1] = isl->xyToVertI(x + 1, y);
            mesh->indices[i + 2] = isl->xyToVertI(x, y);

            // triangle 2
            mesh->indices[i + 3] = isl->xyToVertI(x, y);
            mesh->indices[i + 4] = isl->xyToVertI(x, y + 1);
            mesh->indices[i + 5] = isl->xyToVertI(x + 1, y + 1);

            i += 6;
        }
    }
    mesh->nrOfIndices = mesh->indices.size();
    TangentCalculator::addTangentsToMesh(mesh);
    isl->terrainMesh = mesh;
}
