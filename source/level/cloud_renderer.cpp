
#include "cloud_renderer.h"
#include "graphics/3d/vert_buffer.h"
#include "utils/math_utils.h"
#include "gu/game_utils.h"

const int nrOfSpawnpoints = 300, particlesPerOffset = 6;

CloudRenderer::CloudRenderer(Planet *earth)
    :   earth(earth), quad(new Mesh("cloud_quad", 4, 6, Mesh::getQuad()->attributes)),
        shader(ShaderProgram::fromFiles("CloudsShader", "assets/shaders/clouds.vert", "assets/shaders/clouds.frag")),
        noiseTex(Texture::fromDDSFile("assets/textures/noise.dds"))
{
    quad->vertices = Mesh::getQuad()->vertices;
    quad->indices = Mesh::getQuad()->indices;
    VertBuffer::uploadSingleMesh(quad);

    VertData spawnpoints(VertAttributes()
        .add_({"SPAWNPOINT0", 3})
        .add_({"SPAWNPOINT1", 3})
        .add_({"SPAWNPOINT2", 3})
        .add_({"SPAWNPOINT3", 3}),
        std::vector<float>(nrOfSpawnpoints * 4 * 3)
    );
    for (int i = 0; i < nrOfSpawnpoints; i++)
        for (int j = 0; j < 4; j++)
            spawnpoints.setVec3(
                vec3(mu::random() - .5, (mu::random() - .5) * .5, mu::random() - .5)
                * vec3(1 + i * .15) * vec3(50.),

                i, j * 3
            );

    quad->vertBuffer->uploadPerInstanceData(spawnpoints, particlesPerOffset);
}

void CloudRenderer::render(double time, Camera &cam)
{
    shader.use();

    mat4 t = mat4(1);
    t = rotate(t, 1.f, mu::X);
    t = translate(t, vec3(0, earth->sphere.radius + 35, 0));

    vec3 up = cam.up, right = cam.right;
    up = inverse(t) * vec4(up, 0);
    right = inverse(t) * vec4(right, 0);

    t = cam.combined * t;

    glUniformMatrix4fv(shader.location("mvp"), 1, GL_FALSE, &t[0][0]);
    glUniform3f(shader.location("up"), up.x, up.y, up.z);
    glUniform3f(shader.location("right"), right.x, right.y, right.z);
    glUniform1f(shader.location("time"), time);
    noiseTex->bind(0, shader, "noiseTex");

    glDepthMask(false);
    quad->renderInstances(100 * particlesPerOffset);
    glDepthMask(true);
}
