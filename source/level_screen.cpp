
#include "gu/screen.h"
#include "glad/glad.h"

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
    ShaderProgram shaderProgram;
    FlyingCameraController camController;
    DebugLineRenderer lineRenderer;

    LevelScreen()
        : earth("earth", Sphere(150)),
          cam(PerspectiveCamera(.1, 1000, 1, 1, 75)), camController(&cam),
          shaderProgram(ShaderProgram::fromFiles("NormalTestShader", "gu/assets/shaders/test.vert", "gu/assets/shaders/normaltest.frag"))
    {
        generateEarth(&earth);
        cam.position = glm::vec3(160, 0, 0);
        cam.lookAt(glm::vec3(0));
        cam.update();
    }

    void render(double deltaTime)
    {
        if (KeyInput::justPressed(GLFW_KEY_R))
        {
            earth.destroyIslands();
            earth = Planet("earth", Sphere(150));
            generateEarth(&earth);
        }

        camController.update(deltaTime); // free camera movement

        glClearColor(.3, .1, .6, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glUseProgram(shaderProgram.getProgramId());
        GLuint mvpId = glGetUniformLocation(shaderProgram.getProgramId(), "MVP");

        for (auto isl : earth.islands)
        {
            glm::mat4 mvp = cam.combined * isl->modelInstance->transform;

            glUniformMatrix4fv(mvpId, 1, GL_FALSE, &mvp[0][0]);

            SharedMesh &mesh = isl->model->parts[0].mesh;
            mesh->render();
        }

        glm::mat4 mvp = cam.combined;
        glUniformMatrix4fv(mvpId, 1, GL_FALSE, &mvp[0][0]);
        earth.mesh->render();

        lineRenderer.projection = cam.combined;
        lineRenderer.line(glm::vec3(-300, 0, 0), glm::vec3(300, 0, 0), glm::vec3(1, 0, 0));
        lineRenderer.line(glm::vec3(0, -300, 0), glm::vec3(0, 300, 0), glm::vec3(0, 1, 0));
        lineRenderer.line(glm::vec3(0, 0, -300), glm::vec3(0, 0, 300), glm::vec3(0, 0, 1));

        int normalOffset = earth.mesh->attributes.getOffset(VertAttributes::NORMAL);
        int tangentOffset = earth.mesh->attributes.getOffset(VertAttributes::TANGENT);
        for (int i = 0; i < earth.mesh->nrOfVertices; i++)
        {
            auto p0 = earth.mesh->getVec3(i, 0);
            auto p1 = p0 + earth.mesh->getVec3(i, normalOffset);
            auto p2 = p0 + earth.mesh->getVec3(i, tangentOffset);
            lineRenderer.line(p0, p1, glm::vec3(0, 0, 1));
            lineRenderer.line(p0, p2, glm::vec3(0, 1, 0));
        }

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
