
#include "island.h"
#include "utils/math_utils.h"
#include "input/mouse_input.h"
#include "../serialization.h"
#include "planet.h"
#include "graphics/3d/tangent_calculator.h"

Island::Island(int width, int height, Planet *plt)
    : width(width), height(height), planet(plt),

      nrOfVerts((width + 1) * (height + 1)),
      vertexPositions(nrOfVerts, vec3()),
      vertexNormals(nrOfVerts, vec3()),
      vertexNormalsPlanet(nrOfVerts, vec3()),
      vertexPositionsPlanet(nrOfVerts, vec3()),
      vertexPositionsOriginal(nrOfVerts, vec3()),
      textureMap(nrOfVerts, vec4()),
      buildings(width, std::vector<entt::entity>(height, 0))
{
    std::cout << "Creating island\n";
}

typedef slz::Float<uint32, 200, -200> vertType;
typedef slz::Float<uint8> texType;
typedef slz::Float<uint32, 360, 0> lonLatType;
typedef slz::Float<uint32, 200, -200> outlineType;

Island::Island(const std::vector<uint8> &data, uint32 offs, Planet *plt)
    :
    Island(data.at(offs), data.at(1 + offs), plt)
{
    int nrOfOutlines = data.at(2 + offs);
    longitude = lonLatType::deserialize(data, 3 + offs);
    latitude = lonLatType::deserialize(data, 3 + lonLatType::size + offs);
    seaBottom = vertType::deserialize(data, 3 + lonLatType::size * 2 + offs);

    int startI = 3 + lonLatType::size * 2 + vertType::size + offs;

    vertType::deserializeVecs(data, startI, nrOfVerts, vertexPositionsOriginal, 0);
    assert(vertexPositionsOriginal.size() == nrOfVerts);
    startI += vertType::vecSize<vec3>() * nrOfVerts;

    texType::deserializeVecs(data, startI, nrOfVerts, textureMap, 0);
    startI += texType::vecSize<vec4>() * nrOfVerts;

    for (int i = 0; i < nrOfOutlines; i++)
    {
        auto nrOfPoints = slz::get<uint16>(data, startI);
        startI += 2;
        auto &l = outlines2d.emplace_back();

        outlineType::deserializeVecs(data, startI, nrOfPoints, l.points, 0);

        startI += nrOfPoints * outlineType::vecSize<vec2>();
    }
}

Island::~Island()
{
    std::cout << "Destroying island\n";
}

void Island::toBinary(std::vector<uint8> &out) const
{
    int nrOfOutlinePoints = 0;
    for (auto &l : outlines2d) nrOfOutlinePoints += l.points.size();

    out.reserve(
        3 + // width, height, nr of outlines,
        lonLatType::size * 2 +  // lonlat

        vertType::size + // seaBottom

        vertType::vecSize<vec3>() * nrOfVerts + // vertex positions
        texType::vecSize<vec4>() * nrOfVerts +  // texture map

        outlines2d.size() * 2 +     // nr of points per outline (uint16)
        outlineType::vecSize<vec2>() * nrOfOutlinePoints // outline points
    );
    slz::add((uint8) width, out);
    slz::add((uint8) height, out);
    slz::add((uint8) outlines2d.size(), out);
    slz::add(lonLatType::serialize(longitude), out);
    slz::add(lonLatType::serialize(latitude), out);
    slz::add(vertType::serialize(seaBottom), out);

    vertType::serializeVecs(vertexPositionsOriginal, out);
    texType::serializeVecs(textureMap, out);
    for (auto &l : outlines2d)
    {
        slz::add((uint16) l.points.size(), out);
        outlineType::serializeVecs(l.points, out);
    }
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
    return vertexPositionsOriginal[xyToVertI(x, y)].y <= seaBottom + .1
        && vertexPositionsOriginal[xyToVertI(x + 1, y)].y <= seaBottom + .1
        && vertexPositionsOriginal[xyToVertI(x, y + 1)].y <= seaBottom + .1 
        && vertexPositionsOriginal[xyToVertI(x + 1, y + 1)].y <= seaBottom + .1;
}


bool Island::tileAboveSea(int x, int y)
{
    return vertexPositionsOriginal[xyToVertI(x, y)].y         >= 0
        && vertexPositionsOriginal[xyToVertI(x + 1, y)].y     >= 0
        && vertexPositionsOriginal[xyToVertI(x, y + 1)].y     >= 0
        && vertexPositionsOriginal[xyToVertI(x + 1, y + 1)].y >= 0;
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

bool Island::tileUnderCursor(glm::ivec2 &out, const Camera *cam)
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
            q0 = cam->projectPixels(p0, inViewport),
            q1 = cam->projectPixels(p1, inViewport),
            q2 = cam->projectPixels(p2, inViewport),
            q3 = cam->projectPixels(p3, inViewport);

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

void Island::planetDeform()
{
    float radius = planet->sphere.radius;
    vec3 planetOrigin = vec3(0, -radius, 0);

    // STEP 1: replace vertex positions
    for (int i = 0; i < nrOfVerts; i++)
    {
        vec3 &original = vertexPositionsOriginal[i];
        vec3 &deformed = vertexPositions[i] = normalize(original - planetOrigin);
        deformed *= radius + original.y;
        deformed += planetOrigin;
    }
    // STEP 2: create outlines in 3d.
    for (Polygon &outline : outlines2d)
    {
        outlines3d.push_back(std::vector<vec3>());
        auto &outline3d = outlines3d.back();
        outline3d.reserve(outline.points.size());
        int i = 0;
        for (vec2 &p2D : outline.points)
        {
            vec3 p3D = normalize(vec3(p2D.x, 0, p2D.y) - planetOrigin);
            p3D *= radius;
            p3D += planetOrigin;
            outline3d.push_back(p3D);
        }
    }
}

static int
    lol1[]{0, 1, 0, -1},
    lol2[]{1, 0, -1, 0},
    lol3[]{0, -1, 0, 1};

void Island::calculateNormals()
{
    for (int vertI = 0; vertI < nrOfVerts; vertI++)
    {
        int x = vertIToX(vertI), y = vertIToY(vertI);
        vec3 normal = vec3();
        vec3 &p0 = vertexPositions[vertI];
        for (int i = 0; i < 4; i++)
        {
            int x1 = x + lol1[i];
            int y1 = y + lol2[i];
            int x2 = x + lol2[i];
            int y2 = y + lol3[i];

            if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0 || x1 > width || x2 > width || y1 > height || y2 > height)
                continue;
            vec3
                &p1 = vertexPositions[xyToVertI(x1, y1)],
                &p2 = vertexPositions[xyToVertI(x2, y2)];

            normal += mu::calculateNormal(p0, p1, p2);
        }
        vertexNormals[vertI] = normalize(normal);
        vertexNormalsPlanet[vertI] = planetTransform * vec4(vertexNormals[vertI], 0);
    }
}

void Island::createMesh()
{
    std::string name = planet->name + "_island_" + std::to_string(planet->islands.size());
    VertAttributes attrs = VertAttributes();
    unsigned int
        posOffset = attrs.add(VertAttributes::POSITION),
        norOffset = attrs.add(VertAttributes::NORMAL),
        uvOffset = attrs.add(VertAttributes::TEX_COORDS),
        tanOffset = attrs.add(VertAttributes::TANGENT),
        texOffset = attrs.add({"TEX_BLEND", 4, GL_FALSE});

    SharedMesh mesh = SharedMesh(new Mesh(name + "_mesh", nrOfVerts, width * height * 6, attrs));

    for (int i = 0; i < nrOfVerts; i++)
    {
        mesh->setVec(vertexPositionsPlanet[i], i, posOffset);     // position
        mesh->setVec(vertexNormalsPlanet[i], i, norOffset);       // normal

        mesh->setVec(
            vec2(vertIToX(i), vertIToY(i)) / vec2(50), i, uvOffset    // texCoords
        );

        mesh->setVec(textureMap[i], i, texOffset);    // texture blending
    }

    int i = 0;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (tileAtSeaFloor(x, y)) continue;

            // triangle 1
            mesh->indices[i + 0] = xyToVertI(x + 1, y + 1);
            mesh->indices[i + 1] = xyToVertI(x + 1, y);
            mesh->indices[i + 2] = xyToVertI(x, y);

            // triangle 2
            mesh->indices[i + 3] = xyToVertI(x, y);
            mesh->indices[i + 4] = xyToVertI(x, y + 1);
            mesh->indices[i + 5] = xyToVertI(x + 1, y + 1);

            i += 6;
        }
    }
    mesh->nrOfIndices = mesh->indices.size();
    TangentCalculator::addTangentsToMesh(mesh);
    terrainMesh = mesh;
}

void Island::transformOutlines()
{
    auto &transformed = outlines3dTransformed;
    transformed.clear();

    for (auto &outline : outlines3d)
    {
        transformed.emplace_back();
        for (auto &p : outline) transformed.back().push_back(planetTransform * vec4(p, 1));
    }
}

void Island::transformVertices()
{
    for (int i = 0; i < nrOfVerts; i++)
        vertexPositionsPlanet[i] = planetTransform * vec4(vertexPositions[i], 1);
}

void Island::calculateLatLonOutlines()
{
    auto &lonLatOutlines = outlinesLongLat;
    lonLatOutlines.clear();

    for (auto &outline : outlines3dTransformed)
    {
        lonLatOutlines.push_back(Polygon());
        auto &lonLatoutline = lonLatOutlines.back();

        for (auto &p : outline)
            lonLatoutline.points.push_back(vec2(
                planet->longitude(p.x, p.z),
                planet->latitude(p.y)
            ));
    }
}

void Island::placeOnPlanet()
{
    mat4 transform(1);
    transform = rotate(transform, longitude * mu::DEGREES_TO_RAD, mu::Y);
    transform = rotate(transform, latitude * mu::DEGREES_TO_RAD, mu::X);
    transform = translate(transform, vec3(0, planet->sphere.radius, 0));
    planetTransform = transform;
    transformOutlines();
    calculateLatLonOutlines();
}

float Island::percentageUnderwater() const
{
    float p = 0.;
    for (int i = 0; i < nrOfVerts; i++)
        if (vertexPositionsOriginal[i].y < 0.) p += 1.;
    return p / nrOfVerts;
}

float Island::tileSteepness(int x, int y)
{
    float minHeight = 99999, maxHeight = -99999;
    for (int x0 = x; x0 <= x + 1; x0++)
    {
        for (int y0 = y; y0 <= y + 1; y0++)
        {
            minHeight = min(minHeight, vertexPositionsOriginal[xyToVertI(x0, y0)].y);
            maxHeight = max(maxHeight, vertexPositionsOriginal[xyToVertI(x0, y0)].y);
        }
    }
    return maxHeight - minHeight;
}

entt::entity Island::getBuilding(int x, int y) const
{
    return buildings[x][y];
}
