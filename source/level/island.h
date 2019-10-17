#ifndef ISLAND_H
#define ISLAND_H

#include <vector>
#include <memory>

#include "utils/math/polygon.h"
#include "graphics/3d/mesh.h"
#include "graphics/camera.h"

#include "json.hpp"
#include "../entt.hpp"

class Planet;
class BuildingSystem;

class Island
{

  public:
    Planet *planet;
    int width, height, nrOfVerts;
    float longitude, latitude;
    mat4 planetTransform;

    bool isInView = true;

    float seaBottom = 0;

    SharedMesh terrainMesh;

    Island(int width, int height, Planet *plt);

    // from binary:
    Island(const std::vector<uint8> &data, uint32 dataOffset, Planet *plt);

    ~Island();

    void toBinary(std::vector<uint8> &out) const;

    int xyToVertI(int x, int y);

    int vertIToX(int i);

    int vertIToY(int i);

    bool tileAtSeaFloor(int x, int y);

    bool tileAboveSea(int x, int y);

    float tileSteepness(int x, int y);

    float distToHeight(int x, int y, float minHeight, float maxHeight, int maxDist);

    bool containsLonLatPoint(float lon, float lat);

    bool tileUnderCursor(ivec2 &out, const Camera *cam);

    bool containsTile(int x, int y) const;

    // returns 0-1 based on amount of vertices that are underwater
    float percentageUnderwater() const;

    std::vector<vec3>
        // normal per vertex in local space
        vertexNormals,

        // normal per vertex
        vertexNormalsPlanet,

        // vertex positions in local space, including curvature of the planet.
        vertexPositions,

        // vertex positions in 'planet'/world space, including curvature of the planet.
        vertexPositionsPlanet,

        // vertex positions in local space, EXCLUDING curvature.
        vertexPositionsOriginal;

    std::vector<Polygon>
        // outlines spherically projected on the planet using Longitude & Latitude
        outlinesLongLat,

        // outlines in 2d. (As if the planet was flat)
        outlines2d;

    std::vector<std::vector<vec3>>
        // outlines in 3d.
        outlines3d,
        // outlines in 3d transformed.
        outlines3dTransformed;

    /**
     * Texture map of the island.
     * An island can have 5 textures.
     * 1 background texture and 4 extra textures.
     *
     * Each vertex has a vec4.
     * 
     * The n-th float of that vec4 tells how much of n-th texture should be rendered at the position of that vertex.
     */
    std::vector<vec4> textureMap;

    /**
     * The following functions were previously in (island/planet)_generator.h
     * But these functions are now also needed by the planet_loader
     */
    void planetDeform();

    void calculateNormals();

    void createMesh();

    void transformVertices();

    void placeOnPlanet();

    entt::entity getBuilding(int x, int y) const;

  private:

    mutable std::vector<std::vector<entt::entity>> buildings;
    friend BuildingSystem; // the building system is allowed to place and remove buildings

    ivec2 prevTileUnderCursor = ivec2(0); // used by tileUnderCursor() as optimization

    void transformOutlines();

    void calculateLatLonOutlines();

};

#endif
