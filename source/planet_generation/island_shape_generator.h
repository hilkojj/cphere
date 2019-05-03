
#ifndef ISLAND_SHAPE_GENERATOR
#define ISLAND_SHAPE_GENERATOR

#include "../level/island.h"

/**
 * Generates a boolean list that represents the shape of a island.
 * 
 * example:
 * 
 * .........
 * ...OOO.O.
 * ..OOOOOO.
 * .OOOOOO..
 * ...OO....
 * .........
 *
 * where '.' = sea and 'O' = land
 * 
 * use Island.xyToVertI(x, y) to get the right index for the list.
 */
class IslandShapeGenerator
{
  public:
    IslandShapeGenerator(Island *isl);

    std::vector<bool> shape;

  private:
    Island *isl;

    std::vector<bool> isSea;

    void addCircles();
    void addCircle(int cX, int cY, int radius);
    void removeGaps();
    bool checkIfSea(int x, int y);
};

#endif
