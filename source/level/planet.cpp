
#include "planet.h"
#include "utils/math_utils.h"
#include "glm/gtx/rotate_vector.hpp"

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

bool Planet::cursorToLonLat(const Camera &cam, vec2 &lonLat) const
{
    vec3 rayDir = cam.getCursorRayDirection();
    vec3 intersection;
    if (sphere.rayIntersection(cam.position, rayDir, &intersection, NULL))
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
    if (!cursorToLonLat(cam, lonLat)) return NULL;

    if (lastIslandUnderCursor && lastIslandUnderCursor->containsLonLatPoint(lonLat.x, lonLat.y))
        return lastIslandUnderCursor;

    for (Island *isl : islands) if (isl->containsLonLatPoint(lonLat.x, lonLat.y))
    {
        lastIslandUnderCursor = isl;
        return isl;
    }
    return NULL;
}

