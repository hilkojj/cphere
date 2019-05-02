
#include "gu/screen.h"
#include "glad/glad.h"

#include "level/planet.h"
#include "input/key_input.h"
#include "planet_generation/earth_generator.h"
#include "utils/camera/flying_camera_controller.h"
#include "graphics/shader_program.h"
#include "gu/game_utils.h"
#include "utils/json_model_loader.h"
#include "graphics/3d/vert_buffer.h"

class LevelScreen : public Screen
{

  public:
    Planet earth;
    PerspectiveCamera cam;
    ShaderProgram shaderProgram;
    FlyingCameraController camController;

    LevelScreen()
        : earth("earth", Sphere(150)),
          cam(PerspectiveCamera(.1, 1000, 1, 1, 75)), camController(&cam),
          shaderProgram(ShaderProgram::fromFiles("NormalTestShader", "gu/assets/shaders/test.vert", "gu/assets/shaders/normaltest.frag"))
    {
        cam.position = glm::vec3(0, 0, 3);
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

        glClearColor(.4, .3, .7, 1);
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
    }

    void onResize()
    {
        cam.viewportWidth = gu::widthPixels;
        cam.viewportHeight = gu::heightPixels;
    }
};
