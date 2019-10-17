
#include "cloud_renderer.h"
#include "graphics/3d/vert_buffer.h"
#include "utils/math_utils.h"
#include "utils/math/interpolation.h"
#include "gu/game_utils.h"

const int nrOfSpawnpoints = 30, particlesPerOffset = 6;

CloudRenderer::CloudRenderer(Planet *earth)
    :   earth(earth), quad(new Mesh("cloud_quad", 4, 6, Mesh::getQuad()->attributes)),
        shader(ShaderProgram::fromFiles("CloudsShader", "assets/shaders/clouds.vert", "assets/shaders/clouds.frag")),
        noiseTex(Texture::fromDDSFile("assets/textures/noise.dds"))
{
    quad->vertices = Mesh::getQuad()->vertices;
    quad->indices = Mesh::getQuad()->indices;
    VertBuffer::uploadSingleMesh(quad);

    VertData spawnpoints(VertAttributes()
        .add_({"SPAWNPOINT", 3}),
        std::vector<float>(nrOfSpawnpoints * 4 * 3)
    );
    for (int i = 0; i < nrOfSpawnpoints; i++)
        spawnpoints.setVec(
            vec3(mu::random() - .5, (mu::random() - .5) * .3, mu::random() - .5)
            * vec3(1 + i * .2) * vec3(110.),
            i, 0
        );
    quad->vertBuffer->uploadPerInstanceData(spawnpoints, particlesPerOffset);
}

void CloudRenderer::render(double time, double deltaTime, Camera &cam, vec3 sunDir, Planet *earthhh)
{
    earth = earthhh;
    if (clouds.size() == 0) deltaTime = 30;
    while (clouds.size() < 40)
        clouds.push_back({
            mu::random(360), // lon
            mu::random(mu::random() > .5 ? 40. : 0., mu::random() > .5 ? 120. : 180.), // lat
            mu::random(.7, 1.3), // speed
            0,
            mu::random(30, 60), // time to live
            mu::randomInt(3, 30) // nr of particles
        });

    shader.use();
    glUniform1f(shader.location("time"), time);
    noiseTex->bind(0, shader, "noiseTex");

    glDepthMask(false);

    for (int i = clouds.size() - 1; i >= 0; i--)
    {
        auto &cloud = clouds[i];

        cloud.timeSinceSpawn += deltaTime;
        cloud.timeToDespawn -= deltaTime;

        if (cloud.timeToDespawn <= 0) clouds.erase(clouds.begin() + i);

        cloud.lon += .7 * deltaTime * cloud.speed;
        cloud.lat += .4 * deltaTime * cloud.speed;

        mat4 t = rotate(mat4(1.), cloud.lon * mu::DEGREES_TO_RAD, mu::Y);
        t = rotate(t, cloud.lat * mu::DEGREES_TO_RAD, mu::X);
        t = translate(t, vec3(0, earth->sphere.radius + 35, 0));

        vec3 up = cam.up, right = cam.right;
        up = inverse(t) * vec4(up, 0);
        right = inverse(t) * vec4(right, 0);
        t = cam.combined * t;

        float light = 0;
        light += max(0.f, 1 - Interpolation::circleIn(min(1., cloud.lat / 30.)));
        light += max(0.f, 1 - Interpolation::circleIn(min(1., (180 - cloud.lat) / 30.)));

        light += min(1.f, max(0.0f, dot(rotate(vec3(0, 0, 1), cloud.lon * mu::DEGREES_TO_RAD, mu::Y), sunDir) + .8f));

        glUniformMatrix4fv(shader.location("mvp"), 1, GL_FALSE, &t[0][0]);
        glUniform3f(shader.location("up"), up.x, up.y, up.z);
        glUniform3f(shader.location("right"), right.x, right.y, right.z);
        glUniform1f(shader.location("cloudOpacity"), Interpolation::circleIn(max(0.f, min(cloud.timeToDespawn, cloud.timeSinceSpawn)) / 20.));
        glUniform1f(shader.location("light"), light);
        quad->renderInstances(cloud.spawnPoints * particlesPerOffset);
    }
    glDepthMask(true);
}
