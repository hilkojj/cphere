
#include "space_renderer.h"
#include "graphics/3d/vert_buffer.h"
#include "utils/json_model_loader.h"
#include "glm/gtc/matrix_transform.hpp"
#include "utils/math_utils.h"
#include "gu/game_utils.h"

SpaceRenderer::SpaceRenderer()
    : cubeMapShader(ShaderProgram::fromFiles(
        "SpaceCubeMapShader",
        "assets/shaders/space_box.vert",
        "assets/shaders/space_box.frag"
    )),
    cubeMap(CubeMap::fromDDSFiles({
        "assets/textures/space_cubemap/right.dds",
        "assets/textures/space_cubemap/left.dds",
        "assets/textures/space_cubemap/top.dds",
        "assets/textures/space_cubemap/bottom.dds",
        "assets/textures/space_cubemap/front.dds",
        "assets/textures/space_cubemap/back.dds",
    })),
    sunTexture(Texture::fromDDSFile("assets/textures/sun.dds")),
    sunShader(ShaderProgram::fromFiles(
        "SunShader",
        "assets/shaders/sun.vert",
        "assets/shaders/sun.frag"
    ))
{
    cube = JsonModelLoader::fromUbjsonFile("assets/models/cube.ubj", &VertAttributes().add_(VertAttributes::POSITION))[0]->parts[0].mesh;
    VertBuffer::uploadSingleMesh(cube);
}

void SpaceRenderer::renderBox(const vec3 &sunDir, const Camera &cam)
{
    glDisable(GL_BLEND);
    cubeMapShader.use();
    cubeMap->bind(0);
    glUniform1i(cubeMapShader.location("cubemap"), 0);

    mat4 view = rotate(cam.combined, (float) -atan2(sunDir.z, sunDir.x) + mu::PI * .5f, mu::Y);

    glUniformMatrix4fv(cubeMapShader.location("view"), 1, GL_FALSE, &view[0][0]);
    glCullFace(GL_FRONT);
    glDepthMask(false);
    glDepthFunc(GL_LEQUAL);
    cube->render();
    glCullFace(GL_BACK);
    glDepthMask(true);
    glDepthFunc(GL_LESS);
}

void SpaceRenderer::renderSun(const vec3 &sunDir, const Camera &cam, SharedTexture depth, float time)
{
    sunShader.use();
    sunTexture->bind(0, sunShader, "sun");
    depth->bind(1, sunShader, "depthTex");

    mat4 modelT = translate(translate(mat4(1), cam.position), sunDir * vec3(5));
    modelT = rotate(modelT, (float) -atan2(sunDir.z, sunDir.x) + mu::PI * -.5f, mu::Y);
    
    glUniformMatrix4fv(sunShader.location("mvp"), 1, GL_FALSE, &(cam.combined * modelT)[0][0]);
    glUniform2f(sunShader.location("scrSize"), gu::widthPixels, gu::heightPixels);
    glUniform1f(sunShader.location("time"), time);
    glDisable(GL_DEPTH_TEST);
    Mesh::getQuad()->render();
    glEnable(GL_DEPTH_TEST);
}

