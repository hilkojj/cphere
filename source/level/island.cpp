
#include "island.h"
#include "glm/glm.hpp"
using namespace glm;

Island::Island(int width, int height, Planet *plt)
    : width(width), height(height), planet(plt),

      nrOfVerts((width + 1) * (height + 1)),
      vertexPositions(nrOfVerts, glm::vec3()),
      vertexNormals(nrOfVerts, glm::vec3()),
      vertexPositionsOriginal(nrOfVerts, glm::vec3()),
      textureMap(nrOfVerts, glm::vec4())
{
    std::cout << "Creating island\n";
}

Island::~Island()
{
    if (modelInstance)
    {
        delete modelInstance;
        std::cout << "Destroying island + modelInstance\n";
    }
    else
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
