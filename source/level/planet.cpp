
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
    out = glm::rotate(out, lat * mu::RAD_TO_DEGREES, mu::Z);
    return glm::rotate(out, -lon * mu::RAD_TO_DEGREES, mu::Y);
}

Planet::~Planet()
{
    for (Island *isl : islands)
        delete isl;
    std::cout << "Planet " << name << " and al it's islands destroyed.\n";
}
