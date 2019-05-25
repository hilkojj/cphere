#ifndef SPACE_RENDERER_H
#define SPACE_RENDERER_H

#include "planet.h"
#include "graphics/camera.h"
#include "graphics/shader_program.h"
#include "graphics/cube_map.h"

class SpaceRenderer
{
  public:
    SpaceRenderer();

    void render(double deltaTime, const Camera &cam);

  private:
    SharedMesh cube;
    SharedCubeMap cubeMap;
    ShaderProgram cubeMapShader;
};

#endif
