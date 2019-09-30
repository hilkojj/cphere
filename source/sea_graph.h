
#ifndef GRAPH_H
#define GRAPH_H

#include <memory>

#include "utils/math_utils.h"
#include "level/level.h"

struct Node_;

typedef std::shared_ptr<Node_> Node;

class Node_
{
  public:
    vec3 position;
    vec2 lonLat;

    std::vector<Node> connections;

    bool removed = false;

    int distToCoast;

    Node_(vec3 pos);
};

struct WayPoint
{
    Node originalNode;
    vec3 position;
    vec2 lonLat;
};

class SeaGraph
{
  public:

    Planet *plt;

    SeaGraph(Planet *plt);

    std::vector<Node> nodes;

    Node nearest(const vec2 &lonLat) const;

    bool findPath(const vec2 &lonLat0, const vec2 &lonLat1, std::vector<WayPoint> &path) const;

  private:
    void makeIcoGraph(int subs);

    void cutOutIslands();

    void correctNodePositions();

    void calcNodesLonLat();

    void calcDistToCoast();

    void lowerDistToCoast(Node &n, int stepsLeft);

    void createConnections(std::vector<ivec3> &tris);
};

#endif
