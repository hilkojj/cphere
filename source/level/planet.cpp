
#include "planet.h"
#include "utils/math_utils.h"
#include "glm/gtx/rotate_vector.hpp"
#include "../serialization.h"
#include "utils/gu_error.h"
#include "graphics/3d/vert_buffer.h"

using namespace glm;

Planet::Planet(std::string name, Sphere sphere) : name(name), sphere(sphere)
{
    std::cout << "Planet " << name << " created\n";
}

float Planet::longitude(float x, float z) const
{
    return mu::RAD_TO_DEGREES * std::atan2(z, x) + 180.0f;
}

float Planet::latitude(float y) const
{
    return mu::RAD_TO_DEGREES * glm::acos((y) / sphere.radius);
}

float minAbs(float a, float b)
{
    return abs(a) < abs(b) ? a : b;
}

vec2 Planet::deltaLonLat(vec2 a, vec2 b)
{
    vec2 minP = a.x < b.x ? a : b;
    vec2 maxP = a.x < b.x ? b : a;

    return vec2(
        minAbs(b.x - a.x, ((maxP.x - 360) - minP.x) * (a.x < b.x ? 1 : -1)),

        b.y - a.y
    );
}

glm::vec3 Planet::lonLatTo3d(float lon, float lat, float altitude) const
{
    glm::vec3 out(0, sphere.radius + altitude, 0);
    out = glm::rotate(out, lat * mu::DEGREES_TO_RAD, mu::Z);
    return glm::rotate(out, -lon * mu::DEGREES_TO_RAD, mu::Y);
}

// must be called to delete islands.
// NOTE!!!!: is also called when planet generation restarts
void Planet::destroyIslands()
{
    for (Island *isl : islands) delete isl;
    islands.clear();
}

bool Planet::cursorToLonLat(const Camera *cam, vec2 &lonLat) const
{
    vec3 rayDir = cam->getCursorRayDirection();
    vec3 intersection;
    if (sphere.rayIntersection(cam->position, rayDir, &intersection, NULL))
    {
        lonLat.x = longitude(intersection.x, intersection.z);
        lonLat.y = latitude(intersection.y);
        return true;
    }
    else return false;
}

Island *Planet::islUnderCursor(const Camera &cam)
{
    vec2 lonLat;
    if (!cursorToLonLat(&cam, lonLat)) return NULL;

    if (lastIslandUnderCursor && lastIslandUnderCursor->containsLonLatPoint(lonLat.x, lonLat.y))
        return lastIslandUnderCursor;

    for (Island *isl : islands) if (isl->containsLonLatPoint(lonLat.x, lonLat.y))
    {
        lastIslandUnderCursor = isl;
        return isl;
    }
    return NULL;
}

void Planet::toBinary(std::vector<uint8> &out) const
{
    slz::add((uint8) islands.size(), out);
    for (int i = 0; i < islands.size(); i++)
        slz::add(uint32(0), out);

    for (int i = 0; i < islands.size(); i++)
    {
        uint32 beginSize = out.size();
        islands[i]->toBinary(out);
        uint32 size = out.size() - beginSize;

        memcpy(&out[1 + i * 4], &size, 4);
    }
}

void Planet::fromBinary(const std::vector<uint8> &in, PlanetMeshGenerator meshGenerator, unsigned int inputOffset)
{
    uint8 nrOfIslands = in.at(inputOffset);

    uint32 startI = nrOfIslands * 4 + 1 + inputOffset;
    for (int i = 0; i < nrOfIslands; i++)
    {
        auto size = slz::get<uint32>(in, inputOffset + 1 + i * 4);

        islands.push_back(new Island(in, startI, this));
        auto isl = islands.back();
        isl->planetDeform();
        isl->placeOnPlanet();
        isl->transformVertices();
        isl->calculateNormals();
        isl->createMesh();
        startI += size;
    }
    mesh = meshGenerator();
    uploadMeshes();
    std::cout << "planet loaded from binary\n";
}

void Planet::uploadMeshes()
{
    VertBuffer *buffer = NULL;
    for (auto isl : islands)
    {
        if (!buffer) buffer = VertBuffer::with(isl->terrainMesh->attributes);
        buffer->add(isl->terrainMesh);
    }
    if (buffer) buffer->upload(false);
    VertBuffer::uploadSingleMesh(mesh);
}
