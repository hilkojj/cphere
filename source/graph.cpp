
#include "graph.h"

// todo make not hacky

Node_::Node_(vec3 pos) : position(pos), lonLat(
    vec2(
        mu::RAD_TO_DEGREES * std::atan2(pos.z, pos.x) + 180.0f,

        mu::RAD_TO_DEGREES * glm::acos(pos.y)
    )
)
{
}

int getMiddlePoint(std::map<std::pair<int, int>, int> &cache, std::vector<Node> &nodes, int n0i, int n1i)
{
    int nLowerI = n0i < n1i ? n0i : n1i;
    int nHigherI = n0i < n1i ? n1i : n0i;

    if (cache.count({nLowerI, nHigherI}))
        return cache[{nLowerI, nHigherI}];

    Node n0 = nodes[n0i], n1 = nodes[n1i];

    Node splitted = Node(new Node_((n0->position + n1->position) / vec3(2.)));

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

void Graph::makeIcoGraph(int subs, float radius)
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
        std::vector<ivec3> newTris;

        for (auto &tri : tris)
        {
            Node p0 = nodes[tri.x], p1 = nodes[tri.y], p2 = nodes[tri.z];

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

    // create connections
    std::map<std::pair<int, int>, bool> connectionCreated;

    for (auto &tri : tris)
    {
        createConnection(connectionCreated, nodes, tri.x, tri.y);
        createConnection(connectionCreated, nodes, tri.y, tri.z);
        createConnection(connectionCreated, nodes, tri.z, tri.x);
    }

    for (auto &n : nodes)
    {
        vec3 &p = n->position;
        double length = sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
        n->position /= length;

        n->position *= radius;
    }

}
