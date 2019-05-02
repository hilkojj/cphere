#ifndef ISLAND_OUTLINER_H
#define ISLAND_OUTLINER_H

#include <vector>
#include <optional>

#include "utils/math/polygon.h"
#include "../level/island.h"

struct Intersection
{
    glm::vec2 pos;
    bool checked = false;
    int partnerI = 0;
};

class IslandOutliner
{
  public:
    IslandOutliner(Island *isl, float intersectY);

    // returns outlines of the island.
    void getOutlines(std::vector<Polygon> &outlines);

    // checks if 'outlines' are correct.
    bool checkOutlines(std::vector<Polygon> &outlines);

  private:
    Island *isl;
    float intersectY;
    std::vector<Intersection> intersections;

    void findIntersections();

    std::optional<Intersection> findIntersection(glm::vec3 &p0, glm::vec3 &p1);

    Intersection *firstUnchecked();

    Intersection *findClosest(glm::vec2 &pos);
};

#endif
