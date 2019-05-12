
#include "gu/screen.h"
#include "glad/glad.h"
#include "graphics/texture.h"
#include "level/planet.h"
#include "input/key_input.h"
#include "planet_generation/earth_generator.h"
#include "utils/camera/flying_camera_controller.h"
#include "graphics/shader_program.h"
#include "gu/game_utils.h"
#include "graphics/3d/debug_line_renderer.h"
#include "utils/math_utils.h"

class LevelScreen : public Screen
{

  public:
    Planet earth;
    PerspectiveCamera cam;
    ShaderProgram shaderProgram, earthShader;
    FlyingCameraController camController;
    DebugLineRenderer lineRenderer;
    SharedTexture seaNormalMap;
    SharedTexture seaDUDV;

    float time = 0;

    LevelScreen()
        : earth("earth", Sphere(150)),
          cam(PerspectiveCamera(.1, 1000, 1, 1, 75)), camController(&cam),
          shaderProgram(ShaderProgram::fromFiles("NormalTestShader", "gu/assets/shaders/test.vert", "gu/assets/shaders/normaltest.frag")),
          seaNormalMap(Texture::fromDDSFile("assets/textures/sea_normals.dds")),
          seaDUDV(Texture::fromDDSFile("assets/textures/sea_dudv.dds")),
          earthShader(ShaderProgram::fromFiles("EarthShader", "assets/shaders/earth.vert", "assets/shaders/earth.frag"))
    {
        generateEarth(&earth);
        cam.position = glm::vec3(200, 0, 0);
        cam.lookAt(glm::vec3(0));
        cam.update();
        camController.speedMultiplier = 10;
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    void render(double deltaTime)
    {
        time += deltaTime * (KeyInput::pressed(GLFW_KEY_KP_ADD) ? 10 : 1);
        if (KeyInput::justPressed(GLFW_KEY_R))
        {
            earth.destroyIslands();
            earth = Planet("earth", Sphere(150));
            generateEarth(&earth);
        }

        camController.update(deltaTime); // free camera movement

        glClearColor(.01, .01, .05, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        shaderProgram.use();
        GLuint mvpId = glGetUniformLocation(shaderProgram.getProgramId(), "MVP");

        for (auto isl : earth.islands)
        {
            glm::mat4 mvp = cam.combined * isl->modelInstance->transform;

            glUniformMatrix4fv(mvpId, 1, GL_FALSE, &mvp[0][0]);

            SharedMesh &mesh = isl->model->parts[0].mesh;
            mesh->render();
        }

        earthShader.use();
        seaNormalMap->bind(0);
        seaDUDV->bind(1);
        glm::mat4 mvp = cam.combined;
        glm::vec3 sunDir = glm::vec3(glm::sin(time * .03), 0, glm::cos(time * .03));
        glUniformMatrix4fv(mvpId, 1, GL_FALSE, &mvp[0][0]);
        glUniform1i(glGetUniformLocation(earthShader.getProgramId(), "seaNormals"), 0);
        glUniform1i(glGetUniformLocation(earthShader.getProgramId(), "seaDUDV"), 1);
        glUniform1f(glGetUniformLocation(earthShader.getProgramId(), "time"), time);
        glUniform3f(glGetUniformLocation(earthShader.getProgramId(), "camPos"), cam.position.x, cam.position.y, cam.position.z);
        glUniform3f(glGetUniformLocation(earthShader.getProgramId(), "sunDir"), sunDir.x, sunDir.y, sunDir.z);
        earth.mesh->render();

        lineRenderer.projection = cam.combined;
        lineRenderer.line(glm::vec3(-300, 0, 0), glm::vec3(300, 0, 0), glm::vec3(1, 0, 0));
        lineRenderer.line(glm::vec3(0, -300, 0), glm::vec3(0, 300, 0), glm::vec3(0, 1, 0));
        lineRenderer.line(glm::vec3(0, 0, -300), glm::vec3(0, 0, 300), glm::vec3(0, 0, 1));

        for (int i = 0; i < 100; i += 2)
            lineRenderer.line(sunDir * glm::vec3(i * 5), sunDir * glm::vec3((i + 1) * 5), glm::vec3(1, 1, 0));

        // int normalOffset = earth.mesh->attributes.getOffset(VertAttributes::NORMAL);
        // int tangentOffset = earth.mesh->attributes.getOffset(VertAttributes::TANGENT);
        // for (int i = 0; i < earth.mesh->nrOfVertices; i++)
        // {
        //     auto p0 = earth.mesh->getVec3(i, 0);
        //     auto p1 = p0 + earth.mesh->getVec3(i, normalOffset);
        //     auto p2 = p0 + earth.mesh->getVec3(i, tangentOffset);
        //     lineRenderer.line(p0, p1, glm::vec3(0, 0, 1));
        //     lineRenderer.line(p0, p2, glm::vec3(0, 1, 0));
        // }

        // glClear(GL_DEPTH_BUFFER_BIT);

        // for (auto isl : earth.islands)
        // {
        //     for (auto &outline : isl->outlines3dTransformed)
        //     {
        //         auto *prev = &outline[0];
        //         for (auto &p : outline)
        //         {
        //             lineRenderer.line(*prev, p, glm::vec3(1, 0, 0));
        //             prev = &p;
        //         }
        //     }
        // }
    }

    void onResize()
    {
        cam.viewportWidth = gu::widthPixels;
        cam.viewportHeight = gu::heightPixels;
    }
};
