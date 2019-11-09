#ifndef ISLAND_H
#define ISLAND_H

#include <vector>
#include <memory>

#include "utils/math/polygon.h"
#include "graphics/3d/mesh.h"
#include "graphics/camera.h"

#include "buildings/building.h"

#include "json.hpp"

class Planet;
class BuildingsSystem;

static const float ISLAND_ROTATION = 30;

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

    int xyToVertI(int x, int y) const;

    int vertIToX(int i) const;

    int vertIToY(int i) const;

    bool tileAtSeaFloor(int x, int y) const;

    bool tileAboveSea(int x, int y) const;

    float tileSteepness(int x, int y) const;

    float distToHeight(int x, int y, float minHeight, float maxHeight, int maxDist) const;

    bool containsLonLatPoint(float lon, float lat) const;

    bool tileUnderCursor(ivec2 &out, const Camera *cam) const;

    bool containsTile(int x, int y) const;

    vec3 tileCenter(int x, int y) const;

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

    std::map<Blueprint*, std::vector<Building>> buildingsPerBlueprint;

    Building &getBuilding(int x, int y) const;

  private:

    mutable std::vector<std::vector<Building>> buildingsGrid;
    friend BuildingsSystem; // the building system is allowed to place and remove buildings

    mutable ivec2 prevTileUnderCursor = ivec2(0); // used by tileUnderCursor() as optimization

    void transformOutlines();

    void calculateLatLonOutlines();

};

#endif
