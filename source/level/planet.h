#ifndef PLANET_H
#define PLANET_H

#include <iostream>
#include <vector>

#include "island.h"
#include "utils/math/sphere.h"
#include "graphics/3d/mesh.h"
#include "graphics/camera.h"

#define PlanetMeshGenerator std::function<SharedMesh()>

class Planet
{

  public:
    std::string name;

    Sphere sphere;
    
    std::vector<Island*> islands;

    SharedMesh mesh;

    Planet(std::string name, Sphere sphere);

    float longitude(float x, float z) const;

    float latitude(float y) const;

    static vec2 deltaLonLat(vec2 a, vec2 b);

    glm::vec3 lonLatTo3d(float lon, float lat, float altitude) const;

    bool cursorToLonLat(const Camera *cam, vec2 &lonLat) const;

    Island *islUnderCursor(const Camera &cam);

    // must be called to delete islands. NOTE: is also called when planet generation restarts
    void destroyIslands();

    void toBinary(std::vector<uint8> &out) const;

    void fromBinary(const std::vector<uint8> &in, PlanetMeshGenerator meshGenerator, unsigned int inputOffset = 0);

    void uploadMeshes();

  private:

    Island *lastIslandUnderCursor = NULL; // used by islUnderCursor() as optimization

};


#endif
