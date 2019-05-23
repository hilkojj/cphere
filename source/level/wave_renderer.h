#ifndef WAVE_RENDERER_H
#define WAVE_RENDERER_H

#include <vector>

#include "planet.h"
#include "graphics/shader_program.h"

struct Wave
{
    SharedMesh mesh;
    float timer, timeMultiplier;
};

struct IslandWaves
{
    std::vector<Wave> waves;
    Island *isl;
};


class WaveRenderer
{

  public:
    WaveRenderer(Planet &earth);

    void render(double deltaTime, const glm::mat4 &view);

  private:
    Planet &earth;

    ShaderProgram shader;

    std::vector<IslandWaves> islandWaves;

    void createWavesForIsland(Island *isl);

    void createWavesForOutline(IslandWaves &islWaves, Polygon &outline);

    bool createWave(IslandWaves &islWaves, Polygon &outline, Polygon &offsetted, int waveStart, int waveLength);
};

#endif
