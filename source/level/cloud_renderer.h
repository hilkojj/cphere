#ifndef CLOUD_RENDERER_H
#define CLOUD_RENDERER_H

#include "planet.h"
#include "graphics/camera.h"
#include "graphics/shader_program.h"
#include "graphics/texture.h"

class CloudRenderer
{
  public:
    CloudRenderer(Planet *earth);

    void render(double time, Camera &cam);

  private:
    SharedMesh quad;
    Planet *earth;
    ShaderProgram shader;
    SharedTexture noiseTex;

};

#endif
