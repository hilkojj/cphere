#ifndef GAME_SHADOW_RENDERER_H
#define GAME_SHADOW_RENDERER_H

#include <graphics/texture.h>
#include <utils/math_utils.h>
#include <graphics/frame_buffer.h>
#include <graphics/orthographic_camera.h>

class ShadowRenderer
{
  public:

    SharedTexture sunDepthTexture;
    mat4 shadowMatrix;
    OrthographicCamera sunCam;

    ShadowRenderer();

    void begin(const Camera &mainCam, const vec3 &sunDir);

    void end();

  private:

    FrameBuffer buffer;

};

#endif
