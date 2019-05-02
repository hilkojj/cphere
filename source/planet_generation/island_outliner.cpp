
#include "island_outliner.h"

IslandOutliner::IslandOutliner(Island *isl, float intersectY)
    : isl(isl), intersectY(intersectY)
{
}

void IslandOutliner::getOutlines(std::vector<Polygon> &outlines)
{
    findIntersections();
    while (firstUnchecked())
    {
        outlines.push_back(Polygon());
        Polygon &outline = outlines.back();

        Intersection *curr = firstUnchecked();
        outline.points.push_back(curr->pos);
        Intersection *partner = &intersections[curr->partnerI];
        outline.points.push_back(partner->pos);
        
        while (true)
        {
            curr->checked = true;
            partner = &intersections[curr->partnerI];
            partner->checked = true;

            auto closest = findClosest(partner->pos);
            if (!closest)
            {
                outlines.pop_back();
                break;
            }
            curr = closest;
            partner = &intersections[curr->partnerI];

            float distToFirst = glm::length(partner->pos - outline.points[0]);
            if (distToFirst < .01f)
            {
                curr->checked = true;
                partner->checked = true;
                outline.points.push_back(outline.points[0]);
                break;
            }
            outline.points.push_back(partner->pos);
        }
    }
}

void IslandOutliner::findIntersections()
{
    for (int x = 0; x < isl->width; x++)
    {
        for (int y = 0; y < isl->height; y++)
        {
            std::optional<Intersection> first;
            glm::vec3
                &a = isl->vertexPositionsOriginal[isl->xyToVertI(x, y + 1)],
                &b = isl->vertexPositionsOriginal[isl->xyToVertI(x + 1, y + 1)],
                &c = isl->vertexPositionsOriginal[isl->xyToVertI(x + 1, y)],
                &d = isl->vertexPositionsOriginal[isl->xyToVertI(x, y)];

            for (int i = 0; i < 4; i++)
            {
                auto intersection = findIntersection(
                    i == 0 ? a : (i == 1 ? b : (i == 2 ? c : d)),

                    i == 0 ? b : (i == 1 ? c : (i == 2 ? d : a))
                );
                if (!intersection.has_value())
                    continue;
                if (!first.has_value()) first = intersection;
                else
                {
                    first.value().partnerI = intersections.size() + 1;
                    intersection.value().partnerI = intersections.size();
                    intersections.push_back(first.value());
                    intersections.push_back(intersection.value());
                    break;
                }
            }
        }
    }
}

std::optional<Intersection> IslandOutliner::findIntersection(glm::vec3 &p0, glm::vec3 &p1)
{
    glm::vec3 
        &lowest = p0.y < p1.y ? p0 : p1,
        &heighest = (lowest == p0) ? p1 : p0;

    if (intersectY < lowest.y || intersectY >= heighest.y)
        return {};

    float h = (intersectY - lowest.y) / (heighest.y - lowest.y);
    glm::vec3 p = glm::mix(lowest, heighest, h);
    return Intersection{glm::vec2(p.x, p.z), false};
}

Intersection *IslandOutliner::firstUnchecked()
{
    for (Intersection &p : intersections) if (!p.checked) return &p;
    return NULL;
}

Intersection *IslandOutliner::findClosest(glm::vec2 &pos)
{
    Intersection *closest = NULL;
    float closestDist = 0;

    for (Intersection &p : intersections)
    {
        if (p.checked) continue;

        float dist = glm::length(pos - p.pos);
        if (dist < closestDist || !closest)
        {
            closest = &p;
            closestDist = dist;
        }
    }
    return closest;
}

/**
 * Checks if generated outlines are correct.
 * For each vertex that should be inside an outline -> check if it really is inside an outline
 */
bool IslandOutliner::checkOutlines(std::vector<Polygon> &outlines)
{
    for (int i = 0; i < isl->nrOfVerts; i++)
    {
        glm::vec3 &p = isl->vertexPositionsOriginal[i];
        if (p.y <= intersectY) continue;

        bool valid = false;
        for (Polygon &outline : outlines)
        {
            if (outline.contains(p.x, p.z))
            {
                valid = true;
                break;
            }
        }
        if (!valid) return false;
    }
    return true;
}
