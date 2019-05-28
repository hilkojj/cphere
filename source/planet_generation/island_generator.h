#ifndef ISLAND_GENERATOR_H
#define ISLAND_GENERATOR_H

#include <functional>
#include "../level/island.h"

typedef std::function<void(Island *)> TerrainGenerator;
typedef std::function<void(Island *)> TextureMapper;

class IslandGenerator
{

  public:
    IslandGenerator(int width, int height, Planet *plt, TerrainGenerator terrainGenerator, TextureMapper textureMapper);

    Island *generateEssentials();

    void finishGeneration();

  private:
    Island *isl = nullptr;
    TerrainGenerator terrainGenerator;
    TextureMapper textureMapper;

    bool generated = false;

    void reset(int width, int height, Planet *plt);

    bool tryToGenerate();

    void initVertexPositions();

    void planetDeform();

    void calculateNormals();

    void createMesh();

};

#endif
