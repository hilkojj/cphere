
#include <files/file.h>
#include "utils/math_utils.h"
#include "island_shape_generator.h"

IslandShapeGenerator::IslandShapeGenerator(Island *isl)
    : isl(isl), shape(isl->nrOfVerts, false), isSea(isl->nrOfVerts, false)
{
    addCircles();
    removeGaps();
}

void IslandShapeGenerator::addCircles()
{
    int amount = isl->width * isl->height / 50;
    for (int i = 0; i < amount; i++)
    {
        int maxRadius = glm::min(isl->width, isl->height) / 3;
        if (i > amount * .1f)
            maxRadius *= .5;
        int radius = mu::random(maxRadius / 10, maxRadius / 2);
        int cX = mu::random(radius + 2, isl->width - radius - 2);
        int cY = mu::random(radius + 2, isl->height - radius - 2);
        if (i < amount * .1f || shape[isl->xyToVertI(cX, cY)])
        {

            addCircle(cX, cY, radius);
//            std::vector<unsigned char> out;
//            for (int y = 0; y < isl->height; y++)
//            {
//                for (int x = 0; x < isl->width; x++)
//                {
//                    if (shape[isl->xyToVertI(x, y)])
//                        out.push_back('#');
//                    else out.push_back('_');
//                }
//                out.push_back('\n');
//            }
//            File::writeBinary((std::to_string(i) + ".txt").c_str(), out);
        }

    }
}

void IslandShapeGenerator::addCircle(int cX, int cY, int radius)
{

    for (int x = 0; x <= isl->width; x++)
    {
        for (int y = 0; y <= isl->height; y++)
        {
            int xDiff = x - cX;
            int yDiff = y - cY;

            if (glm::sqrt(xDiff * xDiff + yDiff * yDiff) < radius)
                shape[isl->xyToVertI(x, y)] = true;
        }
    }
}

void IslandShapeGenerator::removeGaps()
{
    isSea[0] = true;
    for (int x = 0; x <= isl->width; x++)
        for (int y = 0; y <= isl->height; y++)
            checkIfSea(x, y);
    for (int x = isl->width; x >= 0; x--)
        for (int y = isl->height; y >= 0; y--)
            if (!checkIfSea(x, y))
                shape[isl->xyToVertI(x, y)] = true;
}

bool IslandShapeGenerator::checkIfSea(int x, int y)
{
    if (shape[isl->xyToVertI(x, y)])
        return false;

    for (int x0 = glm::max(0, x - 1); x0 <= glm::min(isl->width, x + 1); x0++)
    {
        for (int y0 = glm::max(0, y - 1); y0 <= glm::min(isl->height, y + 1); y0++)
        {
            if (x0 != x && y0 != y)
                continue;
            if (isSea[isl->xyToVertI(x0, y0)])
            {
                isSea[isl->xyToVertI(x, y)] = true;
                return true;
            }
        }
    }
    return false;
}