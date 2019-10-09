
#include "sea_graph.h"
#include "utils/a_star_pathfinder.h"

// todo make not hacky

Node_::Node_(vec3 pos) : position(pos)
{
}

SeaGraph::SeaGraph(Planet *plt) : plt(plt) {
}

void SeaGraph::generate()
{
    nodes.clear();
    makeIcoGraph(5);
    cutOutIslands();
}

int getMiddlePoint(std::map<std::pair<int, int>, int> &cache, std::vector<Node> &nodes, int n0i, int n1i)
{
    int nLowerI = n0i < n1i ? n0i : n1i;
    int nHigherI = n0i < n1i ? n1i : n0i;

    if (cache.count({nLowerI, nHigherI}))
        return cache[{nLowerI, nHigherI}];

    Node n0 = nodes[n0i], n1 = nodes[n1i];

    Node splitted = Node(new Node_((n0->position + n1->position) / vec3(2.)));

    splitted->distToCoast = (n0->distToCoast + n1->distToCoast) / 2.;

    vec3 p = splitted->position;
    nodes.push_back(splitted);

    cache[{nLowerI, nHigherI}] = nodes.size() - 1;

    return nodes.size() - 1;
}

void createConnection(std::map<std::pair<int, int>, bool> &connectionCreated, std::vector<Node> &nodes, int n0i, int n1i)
{
    int nLowerI = n0i < n1i ? n0i : n1i;
    int nHigherI = n0i < n1i ? n1i : n0i;

    if (connectionCreated.count({nLowerI, nHigherI}))
        return;

    connectionCreated[{nLowerI, nHigherI}] = true;

    Node n0 = nodes[n0i], n1 = nodes[n1i];

    n0->connections.push_back(n1);
    n1->connections.push_back(n0);
}

void SeaGraph::makeIcoGraph(int subs)
{

    double t = (1.0 + glm::sqrt(5.)) / 2.;

    // create 12 vertices of a icosahedron:
    // see http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
    nodes.push_back(Node(new Node_(vec3(-1,  t,  0)))); // P0
    nodes.push_back(Node(new Node_(vec3( 1,  t,  0)))); // P1
    nodes.push_back(Node(new Node_(vec3(-1, -t,  0)))); // etc....
    nodes.push_back(Node(new Node_(vec3( 1, -t,  0))));

    nodes.push_back(Node(new Node_(vec3( 0, -1,  t))));
    nodes.push_back(Node(new Node_(vec3( 0,  1,  t))));
    nodes.push_back(Node(new Node_(vec3( 0, -1, -t))));
    nodes.push_back(Node(new Node_(vec3( 0,  1, -t))));

    nodes.push_back(Node(new Node_(vec3( t,  0, -1))));
    nodes.push_back(Node(new Node_(vec3( t,  0,  1))));
    nodes.push_back(Node(new Node_(vec3(-t,  0, -1))));
    nodes.push_back(Node(new Node_(vec3(-t,  0,  1))));
    // 12 vert created

    std::vector<ivec3> tris;
    tris.push_back(ivec3(0, 11, 5));
    tris.push_back(ivec3(0, 5, 1));
    tris.push_back(ivec3(0, 1, 7));
    tris.push_back(ivec3(0, 7, 10));
    tris.push_back(ivec3(0, 10, 11)); 
    tris.push_back(ivec3(1, 5, 9));
    tris.push_back(ivec3(5, 11, 4));
    tris.push_back(ivec3(11, 10, 2));
    tris.push_back(ivec3(10, 7, 6));
    tris.push_back(ivec3(7, 1, 8));
    tris.push_back(ivec3(3, 9, 4));
    tris.push_back(ivec3(3, 4, 2));
    tris.push_back(ivec3(3, 2, 6));
    tris.push_back(ivec3(3, 6, 8));
    tris.push_back(ivec3(3, 8, 9));
    tris.push_back(ivec3(4, 9, 5));
    tris.push_back(ivec3(2, 4, 11));
    tris.push_back(ivec3(6, 2, 10));
    tris.push_back(ivec3(8, 6, 7));

    tris.push_back(ivec3(9, 8, 1));

    std::map<std::pair<int, int>, int> cache;

    for (int i = 0; i < subs; i++)
    {
        bool finalStep = i == (subs - 1);
        if (finalStep)
        {
            correctNodePositions();
            createConnections(tris);
            calcDistToCoast();
        }

        std::vector<ivec3> newTris;

        for (auto &tri : tris)
        {
            Node p0 = nodes[tri.x], p1 = nodes[tri.y], p2 = nodes[tri.z];

            if (finalStep && p0->distToCoast > 2)
            {
                newTris.push_back(tri);
                continue;
            }

            int a = getMiddlePoint(cache, nodes, tri.x, tri.y);
            int b = getMiddlePoint(cache, nodes, tri.y, tri.z);
            int c = getMiddlePoint(cache, nodes, tri.z, tri.x);

            newTris.push_back(ivec3(tri.x, a, c));
            newTris.push_back(ivec3(tri.y, b, a));
            newTris.push_back(ivec3(tri.z, c, b));
            newTris.push_back(ivec3(a, b, c));
        }
        tris = newTris;
    }

    // icosphere done

    for (auto &n : nodes) n->connections.clear();
    createConnections(tris);

    correctNodePositions();
    calcDistToCoast();
}

void SeaGraph::correctNodePositions()
{
    for (auto &n : nodes)
    {
        vec3 &p = n->position;
        double length = sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
        n->position /= length;

        n->position *= plt->sphere.radius;
    }
    calcNodesLonLat();
}

void SeaGraph::cutOutIslands()
{
    for (int i = nodes.size() - 1; i >= 0; i--)
    {
        auto &n = nodes[i];
        // check if n is in island
        if (n->distToCoast > 0) continue;

        n->removed = true;
        nodes[i] = nodes.back();
        nodes.pop_back();
    }
    for (auto &n : nodes)
    {
        for (int i = n->connections.size() - 1; i >= 0; i--)
        {
            if (n->connections[i]->removed)
            {
                n->connections[i] = n->connections.back();
                n->connections.pop_back();
            }
        }
    }
    std::cout << "Created sea-icosphere-network of size: " << nodes.size() << "\n";
}

void SeaGraph::calcNodesLonLat()
{
    for (auto &n : nodes)
    {
        n->lonLat.x = plt->longitude(n->position.x, n->position.z);
        n->lonLat.y = plt->latitude(n->position.y);
    }
}

void SeaGraph::calcDistToCoast()
{
    for (auto &n : nodes) n->distToCoast = 5;
    for (auto &n : nodes)
    {
        Island *isl = plt->islAtLonLat(n->lonLat);
        if (!isl) continue;

        n->distToCoast = 0;
        lowerDistToCoast(n, 4);
    }
}

void SeaGraph::lowerDistToCoast(Node &n, int stepsLeft)
{
    if (stepsLeft == 0) return;
    for (auto &nb : n->connections)
    {
        if (nb->distToCoast == 0) continue;
        nb->distToCoast = min(5 - stepsLeft, nb->distToCoast);
        lowerDistToCoast(nb, stepsLeft - 1);
    }
}

void SeaGraph::createConnections(std::vector<ivec3> &tris)
{
    std::map<std::pair<int, int>, bool> connectionCreated;

    for (auto &tri : tris)
    {
        createConnection(connectionCreated, nodes, tri.x, tri.y);
        createConnection(connectionCreated, nodes, tri.y, tri.z);
        createConnection(connectionCreated, nodes, tri.z, tri.x);
    }
}

Node SeaGraph::nearest(const vec2 &lonLat) const
{
    // todo: this is veeeery slow
    Node nearest = NULL;
    float nearestDist;

    for (auto &n : nodes)
    {
        float dist = length(n->lonLat - lonLat);
        if (!nearest || dist < nearestDist)
        {
            nearest = n;
            nearestDist = dist;
        }
    }
    return nearest;
}

bool SeaGraph::findPath(const vec2 &lonLat0, const vec2 &lonLat1, std::vector<WayPoint> &path) const
{
    static std::vector<Node> nodePath;
    nodePath.clear();
    Node begin = nearest(lonLat0), goal = nearest(lonLat1);

    bool success = findAStarPath<Node>(
        begin, goal,
        [&](Node &n) { return length(n->position - goal->position); },
        [](Node &n0, Node &n1) { return float(1); },
        [](Node &n) -> std::vector<Node>& { return n->connections; },

        nodePath
    );

    int stepSize = 5;
    for (int i = 0; i < (nodePath.size() - stepSize) / stepSize; i++)
    {
        std::vector<Node> subNodePath;
        int ni = (nodePath.size() - stepSize) - stepSize * i;
        auto &n = nodePath[ni];

        bool success = findAStarPath<Node>(
            begin, n,
            [&](Node &n) { return length(n->position - n->position); },
            [](Node &n0, Node &n1) { return float(1); },
            [](Node &n) -> std::vector<Node>& { return n->connections; },

            subNodePath
        );
        if (success && subNodePath.size() < ni - 2)
        {
            for (int j = ni + 1; j < nodePath.size(); j++)
                subNodePath.push_back(nodePath[j]);
            nodePath = subNodePath;
        }
    }

    for (auto &n : nodePath) path.push_back({n, n->position, n->lonLat});
    path.back().position = plt->lonLatTo3d(lonLat1.x, lonLat1.y, 0.);
    path.front().position = plt->lonLatTo3d(lonLat0.x, lonLat0.y, 0.);

    for (int i = 0; i < 10; i++)
    {
        for (int j = 1; j < path.size() - 1; j++)
        {
            auto &wp = path[j];
            auto originalPos = wp.position;
            auto originalLonLat = wp.lonLat;

            auto &p = wp.position;
            p = (path[j - 1].position + path[j + 1].position + p) / float(3);

            double length = sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
            p /= length;
            p *= plt->sphere.radius;
            wp.lonLat.x = plt->longitude(p.x, p.z);
            wp.lonLat.y = plt->latitude(p.y);

            if (wp.originalNode->distToCoast <= 2 && plt->islAtLonLat(wp.lonLat))
            {
                wp.lonLat = originalLonLat;
                wp.position = originalPos;
            }
        }
    }

    return success;
}
