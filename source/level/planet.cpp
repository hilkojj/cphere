
#include "planet.h"
#include "utils/math_utils.h"
#include "glm/gtx/rotate_vector.hpp"

Planet::Planet(std::string name, Sphere sphere) : name(name), sphere(sphere)
{
    std::cout << "Planet " << name << " created\n";
}

float Planet::longitude(float x, float z)
{
    return mu::RAD_TO_DEGREES * std::atan2(z, x) + 180.0f;
}

float Planet::latitude(float y)
{
    return mu::RAD_TO_DEGREES * glm::acos((y) / sphere.radius);
}

glm::vec3 Planet::lonLatTo3d(float lon, float lat, float altitude)
{
    glm::vec3 out(0, sphere.radius + altitude, 0);
    out = glm::rotate(out, lat * mu::DEGREES_TO_RAD, mu::Z);
    return glm::rotate(out, -lon * mu::DEGREES_TO_RAD, mu::Y);
}

void Planet::destroyIslands()
{
    for (Island *isl : islands) delete isl;
}
