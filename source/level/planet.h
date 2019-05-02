#ifndef PLANET_H
#define PLANET_H

#include <iostream>
#include <vector>

#include "island.h"
#include "utils/math/sphere.h"
#include "graphics/3d/mesh.h"

class Planet
{

  public:
    std::string name;

    Sphere sphere;
    
    std::vector<Island*> islands;

    SharedMesh mesh;

    Planet(std::string name, Sphere sphere);

    float longitude(float x, float y);

    float latitude(float y);

    glm::vec3 lonLatTo3d(float lon, float lat, float altitude);

    // must be called to delete islands.
    void destroyIslands();

};


#endif
