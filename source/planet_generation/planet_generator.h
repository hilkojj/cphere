#ifndef PLANET_GENERATOR_H
#define PLANET_GENERATOR_H

#include <functional>

#include "../level/planet.h"
#include "../level/island.h"
#include "glm/glm.hpp"
#include "utils/math/polygon.h"
#include "graphics/3d/model.h"
#include "island_generator.h"

#define IslandContextProvider std::function<IslandContext(int islandNr)>

struct IslandContext
{

    IslandGenerator islandGenerator;
    float minLatitude, maxLatitude;

};


class PlanetGenerator
{

  public:

    PlanetGenerator(Planet *planet, IslandContextProvider islContextProvider, int nrOfIslands, PlanetMeshGenerator meshGenerator);

    void generate();

  private:

    PlanetMeshGenerator meshGenerator;
    IslandContextProvider islContextProvider;

    int nrOfIslands, minNrOfIslands;
    Planet *plt;

    bool tryToGenerate();
    
    bool placeOnPlanet(Island *isl, float minLat, float maxLat);

    bool tryToPlaceOnPlanet(Island *isl, float lon, float lat);

    bool overflowsLongitude(Island* isl);

    bool overlaps(Island* isl0, Island* isl1);

};

#endif
