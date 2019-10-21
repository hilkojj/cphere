#include <iostream>
#include "shadow_renderer.h"

ShadowRenderer::ShadowRenderer()
    : buffer(512, 512), sunCam(0, 300, 150, 150)
{
    buffer.addDepthTexture(GL_LINEAR, GL_LINEAR);
    sunDepthTexture = buffer.depthTexture;
}

void ShadowRenderer::begin(const Camera &mainCam, const vec3 &sunDir)
{
    // calculate matrix, sunCam etc.

    vec3 frustumCenter = mainCam.position + mainCam.direction * float(length(mainCam.position) - 130);

    sunCam.position = frustumCenter - sunDir * float(120);
    sunCam.lookAt(frustumCenter);

    sunCam.update();

    buffer.bind();
    glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowRenderer::end()
{
    buffer.unbind();
}
