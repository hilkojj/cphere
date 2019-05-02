#include "island_generator.h"
#include "island_outliner.h"
#include "utils/math_utils.h"
#include "../level/planet.h"
#include "graphics/3d/model_instance.h"

IslandGenerator::IslandGenerator(int width, int height, Planet *plt, TerrainGenerator terrainGenerator, TextureMapper textureMapper)
    : terrainGenerator(terrainGenerator), textureMapper(textureMapper)
{
    reset(width, height, plt);
}

Island *IslandGenerator::generate()
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
    calculateNormals();
    textureMapper(isl);
    createModel();
    return true;
}

void IslandGenerator::initVertexPositions()
{
    for (int i = 0; i < isl->nrOfVerts; i++)
        isl->vertexPositionsOriginal[i] = glm::vec3(isl->vertIToX(i), 0, isl->vertIToY(i));
}

void IslandGenerator::planetDeform()
{
    float radius = isl->planet->sphere.radius;
    glm::vec3 planetOrigin = glm::vec3(isl->width / 2.0f, -radius, isl->height / 2.0f);

    // STEP 1: replace vertex positions
    for (int i = 0; i < isl->nrOfVerts; i++)
    {
        glm::vec3 &original = isl->vertexPositionsOriginal[i];
        glm::vec3 &deformed = isl->vertexPositions[i] = glm::normalize(original - planetOrigin);
        deformed *= radius + original.y;
        deformed += planetOrigin;
    }
    // STEP 2: create outlines in 3d.
    for (Polygon &outline : isl->outlines2d)
    {
        std::vector<glm::vec3> outline3d(outline.points.size());
        isl->outlines3d.push_back(outline3d);
        int i = 0;
        for (glm::vec2 &p2D : outline.points)
        {
            glm::vec3 &p3D = outline3d[i++] = glm::normalize(glm::vec3(p2D.x, 0, p2D.y) - planetOrigin);
            p3D *= radius;
            p3D += planetOrigin;
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
        glm::vec3 normal = glm::vec3();
        glm::vec3 &p0 = isl->vertexPositions[vertI];
        for (int i = 0; i < 4; i++)
        {
            int x1 = x + lol1[i];
            int y1 = y + lol2[i];
            int x2 = x + lol2[i];
            int y2 = y + lol3[i];

            if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0 || x1 > isl->width || x2 > isl->width || y1 > isl->height || y2 > isl->height)
                continue;

            glm::vec3 
                &p1 = isl->vertexPositions[isl->xyToVertI(x1, y1)],
                &p2 = isl->vertexPositions[isl->xyToVertI(x2, y2)];

            normal += mu::calculateNormal(p0, p1, p2);
        }
        isl->vertexNormals[vertI] = glm::normalize(normal);
    }
}

void IslandGenerator::createModel()
{
    std::string name = isl->planet->name + "_island_" + std::to_string(isl->planet->islands.size());
    VertAttributes attrs = VertAttributes();
    unsigned int
        posOffset = attrs.add(VertAttributes::POSITION),
        norOffset = attrs.add(VertAttributes::NORMAL),
        uvOffset = attrs.add(VertAttributes::TEX_COORDS),
        texOffset = attrs.add({"TEX_BLEND", 4, GL_FALSE});

    SharedMesh mesh = SharedMesh(new Mesh(name + "_mesh", isl->nrOfVerts, isl->width * isl->height * 6, attrs));

    for (int i = 0; i < isl->nrOfVerts; i++)
    {
        int meshI = i * attrs.getVertSize();
        glm::vec3 
            &pos = isl->vertexPositions[i],
            &nor = isl->vertexNormals[i];
        
        mesh->vertices[meshI + posOffset] = pos.x - isl->width / 2.0f;
        mesh->vertices[meshI + posOffset + 1] = pos.y;
        mesh->vertices[meshI + posOffset + 2] = pos.z - isl->height / 2.0f;

        mesh->vertices[meshI + norOffset] = nor.x;
        mesh->vertices[meshI + norOffset + 1] = nor.y;
        mesh->vertices[meshI + norOffset + 2] = nor.z;

        mesh->vertices[meshI + uvOffset] = isl->vertIToX(i) / 50.0f;
        mesh->vertices[meshI + uvOffset + 1] = isl->vertIToY(i) / 50.0f;

        glm::vec4 &blend = isl->textureMap[i];

        mesh->vertices[meshI + texOffset] = blend.r;
        mesh->vertices[meshI + texOffset + 1] = blend.g;
        mesh->vertices[meshI + texOffset + 2] = blend.b;
        mesh->vertices[meshI + texOffset + 3] = blend.a;
    }

    int i = 0;
    for (int x = 0; x < isl->width; x++)
    {
        for (int y = 0; y < isl->height; y++)
        {
            if (isl->tileAtSeaFloor(x, y)) continue;

            int vertI = x + y * (isl->width + 1);

            // triangle 1
            mesh->indices[i + 0] = vertI;
            mesh->indices[i + 1] = vertI + isl->width + 1;
            mesh->indices[i + 2] = vertI + 1;

            // triangle 2
            mesh->indices[i + 3] = vertI + 1;
            mesh->indices[i + 4] = vertI + isl->width + 1;
            mesh->indices[i + 5] = vertI + isl->width + 2;

            i += 6;
        }
    }
    mesh->nrOfIndices = mesh->indices.size();
    isl->model = SharedModel(new Model(name));
    isl->model->parts.push_back({mesh});
    isl->modelInstance = new ModelInstance(isl->model);
}
