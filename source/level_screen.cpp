
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
#include "graphics/frame_buffer.h"
#include "utils/math/sphere_mesh_generator.h"
#include "glm/gtx/transform.hpp"

const float EARTH_RADIUS = 150, ATMOSPHERE_RADIUS = 167;

class LevelScreen : public Screen
{

  public:
    Planet earth;
    PerspectiveCamera cam;
    ShaderProgram shaderProgram, earthShader, causticsShader, atmosphereShader;
    FlyingCameraController camController;
    DebugLineRenderer lineRenderer;
    SharedTexture seaNormalMap, seaDUDV, caustics, sand;
    SharedMesh atmosphereMesh;

    FrameBuffer underwaterBuffer;

    float time = 0;

    LevelScreen()
        : earth("earth", Sphere(EARTH_RADIUS)),
          cam(PerspectiveCamera(.1, 1000, 1, 1, 75)), camController(&cam),
          
          seaNormalMap(Texture::fromDDSFile("assets/textures/sea_normals.dds")),
          seaDUDV(Texture::fromDDSFile("assets/textures/sea_dudv.dds")),
          caustics(Texture::fromDDSFile("assets/textures/tc_caustics.dds")),
          sand(Texture::fromDDSFile("assets/textures/tc_sand.dds")),

          shaderProgram(ShaderProgram::fromFiles("NormalTestShader", "gu/assets/shaders/test.vert", "gu/assets/shaders/normaltest.frag")),
          earthShader(ShaderProgram::fromFiles("EarthShader", "assets/shaders/earth.vert", "assets/shaders/earth.frag")),
          causticsShader(ShaderProgram::fromFiles("CausticsShader", "assets/shaders/terrain_caustics.vert", "assets/shaders/terrain_caustics.frag")),
          atmosphereShader(ShaderProgram::fromFiles("EarthAtmosphereShader", "assets/shaders/earth_atmosphere.vert", "assets/shaders/earth_atmosphere.frag")),

          atmosphereMesh(SphereMeshGenerator::generate("earth_atmosphere", ATMOSPHERE_RADIUS, 90, 70, VertAttributes().add_(VertAttributes::POSITION).add_(VertAttributes::NORMAL))),

          underwaterBuffer(FrameBuffer(512, 512))
    {
        underwaterBuffer.addColorTexture(GL_RGB, GL_LINEAR, GL_LINEAR);
        underwaterBuffer.addDepthTexture(GL_LINEAR, GL_LINEAR);

        VertBuffer::uploadSingleMesh(atmosphereMesh);

        generateEarth(&earth);
        cam.position = glm::vec3(0, 0, 200);
        cam.lookAt(glm::vec3(0));
        cam.update();
        camController.speedMultiplier = 20;
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void render(double deltaTime)
    {
        time += deltaTime * (KeyInput::pressed(GLFW_KEY_KP_ADD) ? 10 : 1);
        if (KeyInput::justPressed(GLFW_KEY_R))
        {
            earth.destroyIslands();
            earth = Planet("earth", Sphere(EARTH_RADIUS));
            generateEarth(&earth);
        }

        camController.update(deltaTime); // free camera movement

        glClearColor(.01, .03, .1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 sunDir = glm::vec3(glm::sin(time * .03), 0, glm::cos(time * .03));

        // // RENDER UNDERWATER:
        underwaterBuffer.bind();
        glDisable(GL_BLEND);

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        causticsShader.use();
        caustics->bind(0);
        sand->bind(1);

        glUniform1f(glGetUniformLocation(causticsShader.id(), "time"), time);
        glUniform1i(glGetUniformLocation(causticsShader.id(), "causticsSheet"), 0);
        glUniform1i(glGetUniformLocation(causticsShader.id(), "terrainTexture"), 1);
        glUniform3f(glGetUniformLocation(causticsShader.id(), "sunDir"), sunDir.x, sunDir.y, sunDir.z);

        for (auto isl : earth.islands)
        {
            glUniformMatrix4fv(glGetUniformLocation(causticsShader.id(), "viewTrans"), 1, GL_FALSE, &cam.combined[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(causticsShader.id(), "worldTrans"), 1, GL_FALSE, &isl->modelInstance->transform[0][0]);

            SharedMesh &mesh = isl->model->parts[0].mesh;
            mesh->render();
        }

        underwaterBuffer.unbindCurrent();
        // // DONE RENDERING UNDERWATER

        // // RENDER ISLANDS:
        shaderProgram.use();
        glDisable(GL_BLEND);
        GLuint mvpId = glGetUniformLocation(shaderProgram.id(), "MVP");

        for (auto isl : earth.islands)
        {
            glm::mat4 mvp = cam.combined * isl->modelInstance->transform;

            glUniformMatrix4fv(mvpId, 1, GL_FALSE, &mvp[0][0]);

            SharedMesh &mesh = isl->model->parts[0].mesh;
            mesh->render();
        }
        // // DONE RENDERING ISLANDS

        // // RENDER WATER:
        earthShader.use();
        glEnable(GL_BLEND);
        seaNormalMap->bind(0);
        seaDUDV->bind(1);
        underwaterBuffer.colorTexture->bind(2);
        underwaterBuffer.depthTexture->bind(3);
        glm::mat4 mvp = cam.combined;
    
        glUniformMatrix4fv(glGetUniformLocation(earthShader.id(), "MVP"), 1, GL_FALSE, &mvp[0][0]);
        glUniform1i(glGetUniformLocation(earthShader.id(), "seaNormals"), 0);
        glUniform1i(glGetUniformLocation(earthShader.id(), "seaDUDV"), 1);
        glUniform1i(glGetUniformLocation(earthShader.id(), "underwaterTexture"), 2);
        glUniform1i(glGetUniformLocation(earthShader.id(), "underwaterDepthTexture"), 3);
        glUniform1f(glGetUniformLocation(earthShader.id(), "time"), time);
        glUniform2f(glGetUniformLocation(earthShader.id(), "scrSize"), gu::widthPixels, gu::heightPixels);
        glUniform3f(glGetUniformLocation(earthShader.id(), "camPos"), cam.position.x, cam.position.y, cam.position.z);
        glUniform3f(glGetUniformLocation(earthShader.id(), "sunDir"), sunDir.x, sunDir.y, sunDir.z);
        earth.mesh->render();
        // DONE RENDERING WATER

        // RENDER ATMOSPHERE:
        atmosphereShader.use();
        // glDisable(GL_CULL_FACE);
        // glCullFace(GL_FRONT);
        glDepthMask(false);

        mvp = glm::mat4(1.0f);
        glm::vec3 cP = glm::normalize(cam.position) * earth.sphere.radius;
        float lon = earth.longitude(cP.x, cP.z), lat = earth.latitude(cP.y);
        mvp = glm::rotate(mvp, -(lon + 90) * mu::DEGREES_TO_RAD, mu::Y);
        mvp = glm::rotate(mvp, lat * mu::DEGREES_TO_RAD, mu::X);
        mvp = cam.combined * mvp;

        glUniformMatrix4fv(glGetUniformLocation(atmosphereShader.id(), "MVP"), 1, GL_FALSE, &mvp[0][0]);
        glUniform3f(glGetUniformLocation(atmosphereShader.id(), "sunDir"), sunDir.x, sunDir.y, sunDir.z);
        glUniform1f(glGetUniformLocation(atmosphereShader.id(), "camDist"), glm::length(cam.position) - ATMOSPHERE_RADIUS);
        
        atmosphereMesh->render();
        glDepthMask(true);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        // DONE RENDERING ATMOSPHERE

        lineRenderer.projection = cam.combined;
        lineRenderer.line(glm::vec3(-300, 0, 0), glm::vec3(300, 0, 0), mu::X);
        lineRenderer.line(glm::vec3(0, -300, 0), glm::vec3(0, 300, 0), mu::Y);
        lineRenderer.line(glm::vec3(0, 0, -300), glm::vec3(0, 0, 300), mu::Z);

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
