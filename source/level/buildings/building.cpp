
#include <utils/math_utils.h>
#include "building.h"
#include "../level.h"

Building_::Building_(Blueprint *bp, int x, int y, int rotation, Island *isl)

    : bp(bp), tiles(bp->width * bp->height)
{
    move(x, y, rotation, isl);
}


void Building_::move(int x, int y, int rotation, Island *isl)
{
    int i = 0;
    bool rot = rotation % 2 == 0;
    for (int x0 = 0; x0 < bp->width; x0++)
        for (int y0 = 0; y0 < bp->height; y0++)
            tiles[i++] = ivec2(x + (rot ? x0 : y0), y + (rot ? y0 : x0));

    this->rotation = rotation;
    this->isl = isl;
    if (isl)
    {
        this->transform = mat4(1);

        vec3 center(0);
        for (auto &t : tiles) center += isl->tileCenter(t.x, t.y);
        center /= tiles.size();

        vec3 normal = normalize(center);

        vec3 centerOnIsl = normal * isl->planet->sphere.radius;

        float longitude = isl->planet->longitude(centerOnIsl.x, centerOnIsl.z);
        float latitude = isl->planet->latitude(centerOnIsl.y);

        transform = rotate(transform, (-longitude - float(90)) * mu::DEGREES_TO_RAD, mu::Y);
        transform = rotate(transform, latitude * mu::DEGREES_TO_RAD, mu::X);
        transform = translate(transform, vec3(0, length(center), 0));
    }
}
