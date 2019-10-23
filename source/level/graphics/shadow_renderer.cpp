#include <iostream>
#include "shadow_renderer.h"

ShadowRenderer::ShadowRenderer()
    : buffer(2048, 2048), sunCam(0, 300, 150, 150)
{
    buffer.addDepthTexture(GL_LINEAR, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    sunDepthTexture = buffer.depthTexture;
}

void ShadowRenderer::begin(const Camera &mainCam, const vec3 &sunDir)
{
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
