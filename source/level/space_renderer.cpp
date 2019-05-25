
#include "space_renderer.h"
#include "graphics/3d/vert_buffer.h"
#include "utils/json_model_loader.h"
#include "glm/gtc/matrix_transform.hpp"

SpaceRenderer::SpaceRenderer()
    : cubeMapShader(ShaderProgram::fromFiles(
        "SpaceCubeMapShader",
        "assets/shaders/space_box.vert",
        "assets/shaders/space_box.frag"
    )),
    cubeMap(CubeMap::fromDDSFiles({
        "assets/textures/space_cubemap/front.dds",
        "assets/textures/space_cubemap/right.dds",
        "assets/textures/space_cubemap/left.dds",
        "assets/textures/space_cubemap/top.dds",
        "assets/textures/space_cubemap/bottom.dds",
        "assets/textures/space_cubemap/back.dds"
    }))
{
    cube = JsonModelLoader::fromUbjsonFile("assets/models/cube.ubj", &VertAttributes().add_(VertAttributes::POSITION))[0]->parts[0].mesh;
    VertBuffer::uploadSingleMesh(cube);
}

void SpaceRenderer::render(double deltaTime, const Camera &cam)
{
    glDisable(GL_BLEND);
    cubeMapShader.use();
    cubeMap->bind(0);
    glUniform1i(cubeMapShader.location("cubemap"), 0);

    mat4 view = mat4(mat3(cam.combined));

    glUniformMatrix4fv(cubeMapShader.location("view"), 1, GL_FALSE, &view[0][0]);
    glCullFace(GL_FRONT);
    glDepthMask(false);
    glDepthFunc(GL_EQUAL);
    cube->render();
    glCullFace(GL_BACK);
    glDepthMask(true);
    glDepthFunc(GL_LESS);
}

