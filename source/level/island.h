#ifndef ISLAND_H
#define ISLAND_H

#include <vector>
#include <memory>

#include "utils/math/polygon.h"
#include "graphics/3d/model.h"
#include "graphics/3d/model_instance.h"

class Planet;

class Island
{

  public:
    Planet *planet;
    int width, height, nrOfVerts;
    float longitude, latitude;

    SharedModel model;
    ModelInstance *modelInstance = nullptr;

    Island(int width, int height, Planet *plt);

    // deletes modelInstance:
    ~Island();

    float seaBottom = 0;

    int xyToVertI(int x, int y);

    int vertIToX(int i);

    int vertIToY(int i);

    bool tileAtSeaFloor(int x, int y);

    float distToHeight(int x, int y, float minHeight, float maxHeight, int maxDist);

    std::vector<glm::vec3>
        // normal per vertex
        vertexNormals,

        // vertex positions in 3d, including curvature of the planet.
        vertexPositions,

        // vertex positions in 3d, excluding curvature.
        vertexPositionsOriginal;

    std::vector<Polygon>
        // outlines spherically projected on the planet using Longitude & Latitude
        outlinesLongLat,

        // outlines in 2d. (As if the planet was flat)
        outlines2d;

    std::vector<std::vector<glm::vec3>>
        // outlines in 3d.
        outlines3d,
        // outlines in 3d transformed.
        outlines3dTransformed;

    /**
     * Texture map of the island.
     * An island can have 5 textures.
     * 1 background texture and 4 additional textures.
     *
     * Each vertex has a vec4.
     * 
     * The n-th float of that vec4 tells how much of n-th texture should be rendered at the position of that vertex.
     */
    std::vector<glm::vec4> textureMap;
};

#endif
