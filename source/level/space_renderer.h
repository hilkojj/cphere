#ifndef SPACE_RENDERER_H
#define SPACE_RENDERER_H

#include "planet.h"
#include "graphics/camera.h"
#include "graphics/shader_program.h"
#include "graphics/cube_map.h"
#include "graphics/3d/model.h"
#include "graphics/texture.h"
#include "graphics/texture_array.h"

struct LensFlare
{
    int texture;
    vec4 color;
    float scale, dist;
    bool rotate;
};

class SpaceRenderer
{
  public:
    SpaceRenderer();

    void renderBox(const vec3 &sunDir, const Camera &cam);

    void renderSun(const vec3 &sunDir, const Camera &cam, SharedTexture depth, float time, const Planet &plt);

  private:

    static const LensFlare flares[];

    SharedTexture sunTexture;

    SharedTexArray flareTextures;

    SharedMesh cube;
    SharedCubeMap cubeMap;
    ShaderProgram cubeMapShader, sunShader, flareShader;

    float lensFlareAlpha = 0;
};

#endif
