#ifndef SPACE_RENDERER_H
#define SPACE_RENDERER_H

#include "planet.h"
#include "graphics/camera.h"
#include "graphics/shader_program.h"
#include "graphics/cube_map.h"
#include "graphics/3d/model.h"
#include "graphics/texture.h"

class SpaceRenderer
{
  public:
    SpaceRenderer();

    void renderBox(const vec3 &sunDir, const Camera &cam);

    void renderSun(const vec3 &sunDir, const Camera &cam, SharedTexture depth, float time);

  private:
    SharedTexture sunTexture;

    SharedMesh cube;
    SharedCubeMap cubeMap;
    ShaderProgram cubeMapShader, sunShader;
};

#endif
