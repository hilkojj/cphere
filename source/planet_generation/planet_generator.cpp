
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
        if (++failedTries > 10)
        {
            minNrOfIslands--;
            failedTries = 0;
        }
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
        Island *isl = context.islandGenerator.generate();

        if (placeOnPlanet(isl, context.minLatitude, context.maxLatitude)) failedTries = 0;
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
        if (++failedTries >= 20) return false;
    }
}

bool PlanetGenerator::tryToPlaceOnPlanet(Island *isl, float lon, float lat)
{
    isl->longitude = lon;
    isl->latitude = lat;
    transformOutlines(isl, lon, lat);
    calculateLatLonOutlines(isl);

    if (overflowsLongitude(isl)) return false;

    for (Island *isl1 : plt->islands)
        if (overlaps(isl, isl1)) return false;

    isl->modelInstance->rotateY(lon);
    isl->modelInstance->rotateX(lat);
    isl->modelInstance->translate(0, plt->sphere.radius, 0);

    plt->islands.push_back(isl);
    return true;
}

void PlanetGenerator::transformOutlines(Island *isl, float lon, float lat)
{
    auto &transformed = isl->outlines3dTransformed;
    transformed.clear();

    for (auto &outline : isl->outlines3d)
    {
        transformed.push_back(std::vector<glm::vec3>());
        auto &transformedOutline = transformed.back();

        for (auto &p : outline)
        {
            glm::vec3 newP = glm::vec3(p);
            newP.x -= isl->width / 2.0f;
            newP.z -= isl->height / 2.0f;
            newP.y += plt->sphere.radius;
            transformedOutline.push_back(
                glm::rotate(
                    glm::rotate(newP, lat * mu::DEGREES_TO_RAD, mu::X),
                    lon * mu::DEGREES_TO_RAD, mu::Y
                )
            );
        }
    }
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
            lonLatoutline.points.push_back(glm::vec2(
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
                dist = glm::abs(prevLon - lon),
                minLon = glm::min(lon, prevLon), maxLon = glm::max(lon, prevLon),
                distWithOverflow = glm::min(dist, minLon + 360 - maxLon);

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
        SharedMesh &mesh = isl->model->parts[0].mesh;
        if (!buffer) buffer = VertBuffer::with(mesh->attributes);
        buffer->add(mesh);
    }
    buffer->upload(false);

    buffer = VertBuffer::with(plt->mesh->attributes);
    buffer->add(plt->mesh)->upload(false);
}
