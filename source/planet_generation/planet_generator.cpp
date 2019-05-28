
#include <iostream>
#include "glm/gtx/rotate_vector.hpp"
#include "planet_generator.h"
#include "utils/math_utils.h"
#include "graphics/3d/vert_buffer.h"

PlanetGenerator::PlanetGenerator(Planet *planet, IslandContextProvider islContextProvider, int nrOfIslands, PlanetMeshGenerator meshGenerator)
    : plt(planet),
      islContextProvider(islContextProvider), 
      nrOfIslands(nrOfIslands), minNrOfIslands(nrOfIslands),
      meshGenerator(meshGenerator)
{
}

void PlanetGenerator::generate()
{
    std::cout << "Starting generation of " << plt->name << std::endl;

    int failedTries = 0;

    while (!tryToGenerate())
    {
        std::cout << "Planet generation failed\n";
        if (++failedTries > 10)
        {
            std::cout << "nr of Islands is too high\n";
            minNrOfIslands--;
            failedTries = 0;
        }
        plt->destroyIslands();
    }
    plt->mesh = meshGenerator();
    uploadMeshes();

    std::cout << "Generation of " << plt->name << " done. Placed " << plt->islands.size() << "/" << nrOfIslands << " islands.\n";
}

bool PlanetGenerator::tryToGenerate()
{
    int maxTries = 10;
    int failedTries = 0;

    while (plt->islands.size() < nrOfIslands && failedTries < maxTries)
    {
        auto context = islContextProvider();
        Island *isl = context.islandGenerator.generateEssentials();

        if (placeOnPlanet(isl, context.minLatitude, context.maxLatitude))
        {
            failedTries = 0;
            context.islandGenerator.finishGeneration();
        }
        else 
        {
            failedTries++;
            delete isl; // so sad
        }
    }
    return plt->islands.size() >= minNrOfIslands;
}

bool PlanetGenerator::placeOnPlanet(Island *isl, float minLat, float maxLat)
{
    int failedTries = 0;
    while (true)
    {
        float lon = mu::random(360), lat = mu::random(minLat, maxLat);

        if (tryToPlaceOnPlanet(isl, lon, lat)) return true;
        if (++failedTries >= 50) return false;
    }
}

bool PlanetGenerator::tryToPlaceOnPlanet(Island *isl, float lon, float lat)
{
    isl->longitude = lon;
    isl->latitude = lat;
    mat4 transform(1);
    transform = rotate(transform, isl->longitude * mu::DEGREES_TO_RAD, mu::Y);
    transform = rotate(transform, isl->latitude * mu::DEGREES_TO_RAD, mu::X);
    transform = translate(transform, vec3(0, 150, 0));
    isl->planetTransform = transform;
    transformOutlines(isl);
    calculateLatLonOutlines(isl);

    if (overflowsLongitude(isl)) return false;

    for (Island *isl1 : plt->islands)
        if (overlaps(isl, isl1)) return false;

    transformVertices(isl);
    plt->islands.push_back(isl);
    return true;
}

void PlanetGenerator::transformOutlines(Island *isl)
{
    auto &transformed = isl->outlines3dTransformed;
    transformed.clear();

    for (auto &outline : isl->outlines3d)
    {
        transformed.push_back(std::vector<vec3>());

        for (auto &p : outline) transformed.back().push_back(isl->planetTransform * vec4(p, 1));
    }
}

void PlanetGenerator::transformVertices(Island *isl)
{
    for (int i = 0; i < isl->nrOfVerts; i++)
        isl->vertexPositionsPlanet[i] = isl->planetTransform * vec4(isl->vertexPositions[i], 1);
}

void PlanetGenerator::calculateLatLonOutlines(Island *isl)
{
    auto &lonLatOutlines = isl->outlinesLongLat;
    lonLatOutlines.clear();

    for (auto &outline : isl->outlines3dTransformed)
    {
        lonLatOutlines.push_back(Polygon());
        auto &lonLatoutline = lonLatOutlines.back();

        for (auto &p : outline)
            lonLatoutline.points.push_back(vec2(
                plt->longitude(p.x, p.z),
                plt->latitude(p.y)
            ));
    }
}

bool PlanetGenerator::overflowsLongitude(Island *isl)
{
    auto &outlines = isl->outlinesLongLat;
    for (auto &outline : outlines)
    {
        float prevLon = outline.points[0].x;
        for (int i = 1; i < outline.points.size(); i++)
        {
            float 
                lon = outline.points[i].x,
                dist = abs(prevLon - lon),
                minLon = min(lon, prevLon), maxLon = max(lon, prevLon),
                distWithOverflow = min(dist, minLon + 360 - maxLon);

            if (distWithOverflow != dist)
                return true;

            prevLon = lon;
        }
    }
    return false;
}

bool PlanetGenerator::overlaps(Island *isl0, Island *isl1)
{
    for (auto &outline0 : isl0->outlinesLongLat)
    {
        for (auto &outline1 : isl1->outlinesLongLat)
        {
            for (auto &p : outline1.points)
                if (outline0.contains(p.x, p.y)) return true;

            for (auto &p : outline0.points)
                if (outline1.contains(p.x, p.y)) return true;
        }
    }
    return false;
}

void PlanetGenerator::uploadMeshes()
{
    VertBuffer *buffer = NULL;
    for (auto isl : plt->islands)
    {
        if (!buffer) buffer = VertBuffer::with(isl->terrainMesh->attributes);
        buffer->add(isl->terrainMesh);
    }
    if (buffer) buffer->upload(false);
    VertBuffer::uploadSingleMesh(plt->mesh);
}
