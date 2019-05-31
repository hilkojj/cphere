
#include "island.h"
#include "utils/math_utils.h"
#include "input/mouse_input.h"

Island::Island(int width, int height, Planet *plt)
    : width(width), height(height), planet(plt),

      nrOfVerts((width + 1) * (height + 1)),
      vertexPositions(nrOfVerts, vec3()),
      vertexNormals(nrOfVerts, vec3()),
      vertexNormalsPlanet(nrOfVerts, vec3()),
      vertexPositionsPlanet(nrOfVerts, vec3()),
      vertexPositionsOriginal(nrOfVerts, vec3()),
      textureMap(nrOfVerts, vec4())
{
    std::cout << "Creating island\n";
}

Island::~Island()
{
    std::cout << "Destroying island\n";
}

int Island::xyToVertI(int x, int y)
{
    return y * (width + 1) + x;
}

int Island::vertIToX(int i)
{
    return i % (width + 1);
}

int Island::vertIToY(int i)
{
    return i / (width + 1);
}

bool Island::tileAtSeaFloor(int x, int y)
{
    return vertexPositionsOriginal[xyToVertI(x, y)].y == seaBottom 
        && vertexPositionsOriginal[xyToVertI(x + 1, y)].y == seaBottom 
        && vertexPositionsOriginal[xyToVertI(x, y + 1)].y == seaBottom 
        && vertexPositionsOriginal[xyToVertI(x + 1, y + 1)].y == seaBottom;
}

float Island::distToHeight(int x, int y, float minHeight, float maxHeight, int maxDist)
{
    float dist = maxDist;
    for (int x0 = max(0, x - maxDist); x0 <= min(width, x + maxDist); x0++)
    {
        for (int y0 = max(0, y - maxDist); y0 <= min(height, y + maxDist); y0++)
        {
            float height = vertexPositionsOriginal[xyToVertI(x0, y0)].y;
            if (height >= minHeight && height <= maxHeight)
            {
                int xDiff = x - x0;
                int yDiff = y - y0;

                dist = min(dist, (float) sqrt(xDiff * xDiff + yDiff * yDiff));
            }
        }
    }
    return dist;
}

bool Island::containsLonLatPoint(float lon, float lat)
{
    for (auto &outline : outlinesLongLat) if (outline.contains(lon, lat)) return true;
    return false;
}

bool Island::containsTile(int x, int y) const
{
    return x > 0 && y > 0 && x < width && y < height;
}

bool Island::tileUnderCursor(glm::ivec2 &out, const Camera &cam)
{
    vec2 cursor = vec2(MouseInput::mouseX, MouseInput::mouseY);
    bool found = false;

    mu::spiral(max(width, height) * 2, [&](ivec2 d) {

        int x = prevTileUnderCursor.x + d.x,
            y = prevTileUnderCursor.y + d.y;

        if (!containsTile(x, y)) return true;

        vec3
            &p0 = vertexPositionsPlanet[xyToVertI(x, y)],
            &p1 = vertexPositionsPlanet[xyToVertI(x + 1, y)],
            &p2 = vertexPositionsPlanet[xyToVertI(x, y + 1)],
            &p3 = vertexPositionsPlanet[xyToVertI(x + 1, y + 1)];

        bool inViewport = false;
        vec3
            q0 = cam.projectPixels(p0, inViewport),
            q1 = cam.projectPixels(p1, inViewport),
            q2 = cam.projectPixels(p2, inViewport),
            q3 = cam.projectPixels(p3, inViewport);

        if (!inViewport) return true;

        if (mu::pointInTriangle(cursor, q0, q2, q1) || mu::pointInTriangle(cursor, q2, q3, q1))
        {
            out = ivec2(x, y);
            prevTileUnderCursor = out;
            found = true;
            return false;
        }
        return true;
    });
    return found;
}
