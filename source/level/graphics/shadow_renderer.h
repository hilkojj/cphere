#ifndef GAME_SHADOW_RENDERER_H
#define GAME_SHADOW_RENDERER_H

#include <graphics/texture.h>
#include <utils/math_utils.h>
#include <graphics/frame_buffer.h>
#include <graphics/orthographic_camera.h>

class ShadowRenderer
{
  public:

    inline static const mat4 BIAS_MATRIX = mat4(
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0
    );

    SharedTexture sunDepthTexture;
    OrthographicCamera sunCam;

    ShadowRenderer();

    void begin(const Camera &mainCam, const vec3 &sunDir);

    void end();

  private:

    FrameBuffer buffer;

};

#endif
