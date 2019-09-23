
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

    Node_(vec3 pos);
};

class Graph
{
  public:

    std::vector<Node> nodes;

    void makeIcoGraph(int subs, float radius);
};

#endif
