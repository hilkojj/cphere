#ifndef CLOUD_RENDERER_H
#define CLOUD_RENDERER_H

#include "../planet.h"
#include "graphics/camera.h"
#include "graphics/shader_program.h"
#include "graphics/texture.h"

struct Cloud
{
    float lon, lat, speed, timeSinceSpawn, timeToDespawn;
    int spawnPoints;
};

class CloudRenderer
{
  public:
    CloudRenderer(Planet *earth);

    void render(double time, double deltaTime, Camera &cam, vec3 sunDir, Planet *earth);

  private:
    SharedMesh quad;
    Planet *earth;
    ShaderProgram shader;
    SharedTexture noiseTex;
    std::vector<Cloud> clouds;

};

#endif
