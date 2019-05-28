
#include "gu/screen.h"
#include "glad/glad.h"
#include "graphics/texture.h"
#include "graphics/texture_array.h"
#include "level/planet.h"
#include "level/wave_renderer.h"
#include "input/key_input.h"
#include "input/mouse_input.h"
#include "planet_generation/earth_generator.h"
#include "utils/camera/flying_camera_controller.h"
#include "graphics/shader_program.h"
#include "gu/game_utils.h"
#include "graphics/3d/debug_line_renderer.h"
#include "utils/math_utils.h"
#include "graphics/frame_buffer.h"
#include "utils/math/sphere_mesh_generator.h"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/rotate_vector.hpp"

const float EARTH_RADIUS = 150, ATMOSPHERE_RADIUS = 185;

class LevelScreen : public Screen
{

  public:
    Planet earth;
    PerspectiveCamera cam;
    ShaderProgram earthShader, causticsShader, terrainShader, atmosphereShader;
    FlyingCameraController camController;
    DebugLineRenderer lineRenderer;
    SharedTexture seaNormalMap, seaDUDV, caustics, sand, foamTexture;
    SharedTexArray terrainTextures;
    SharedMesh atmosphereMesh;

    FrameBuffer underwaterBuffer;
    WaveRenderer *waveRenderer;

    float time = 0;

    LevelScreen()
        : earth("earth", Sphere(EARTH_RADIUS)),
          cam(PerspectiveCamera(.1, 1000, 1, 1, 55)), camController(&cam),
          
          seaNormalMap(Texture::fromDDSFile("assets/textures/sea_normals.dds")),
          seaDUDV(Texture::fromDDSFile("assets/textures/sea_dudv.dds")),
          caustics(Texture::fromDDSFile("assets/textures/tc_caustics.dds")),
          sand(Texture::fromDDSFile("assets/textures/tc_sand.dds")),
          foamTexture(Texture::fromDDSFile("assets/textures/tc_foam.dds")),

          terrainTextures(TextureArray::fromDDSFiles({
              "assets/textures/tc_sand.dds",
              "assets/textures/tc_sand_normal.dds",

              "assets/textures/tc_grass.dds",
              "assets/textures/tc_grass_dead.dds",
          })),

          earthShader(ShaderProgram::fromFiles("EarthShader", "assets/shaders/earth.vert", "assets/shaders/earth.frag")),
          atmosphereShader(ShaderProgram::fromFiles("EarthAtmosphereShader", "assets/shaders/earth_atmosphere.vert", "assets/shaders/earth_atmosphere.frag")),
          causticsShader(ShaderProgram::fromFiles("CausticsShader", "assets/shaders/terrain_caustics.vert", "assets/shaders/terrain_caustics.frag")),
          terrainShader(ShaderProgram::fromFiles("TerrainShader", "assets/shaders/terrain.vert", "assets/shaders/terrain.frag")),

          atmosphereMesh(SphereMeshGenerator::generate("earth_atmosphere", ATMOSPHERE_RADIUS, 60, 70, VertAttributes().add_(VertAttributes::POSITION).add_(VertAttributes::NORMAL))),

          underwaterBuffer(FrameBuffer(512, 512))
    {
        underwaterBuffer.addColorTexture(GL_RGBA, GL_LINEAR, GL_LINEAR);
        underwaterBuffer.addDepthTexture(GL_LINEAR, GL_LINEAR);

        VertBuffer::uploadSingleMesh(atmosphereMesh);

        generateEarth(&earth);
        waveRenderer = new WaveRenderer(earth);
        cam.position = glm::vec3(280, 90, 280);
        cam.lookAt(glm::vec3(0));
        cam.update();
        camController.speedMultiplier = 100;
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void render(double deltaTime)
    {
        double newDeltaTime =  deltaTime * (KeyInput::pressed(GLFW_KEY_KP_ADD) ? 10 : 1);
        time += newDeltaTime;
        if (KeyInput::justPressed(GLFW_KEY_R))
        {
            earth.destroyIslands();
            earth = Planet("earth", Sphere(EARTH_RADIUS));
            generateEarth(&earth);
            delete waveRenderer;
            waveRenderer = new WaveRenderer(earth);
        }

        glClearColor(.01, .03, .1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Island *hoveredIsland = NULL;

        if (KeyInput::justPressed(GLFW_KEY_G))
        {
            MouseInput::setLockedMode(false);
        }
        else if (KeyInput::justReleased(GLFW_KEY_G))
        {
            MouseInput::setLockedMode(true);
        }
        if (!KeyInput::pressed(GLFW_KEY_G))
            camController.update(deltaTime); // free camera movement
        else
        {
            hoveredIsland = earth.islUnderCursor(cam);
        }

        glm::vec3 sunDir = glm::vec3(glm::sin(time * .03), 0, glm::cos(time * .03));

        // RENDER UNDERWATER:
        underwaterBuffer.bind();
        glEnable(GL_BLEND);

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        causticsShader.use();
        caustics->bind(0);
        sand->bind(1);

        glUniform1f(causticsShader.location("time"), time);
        glUniform1i(causticsShader.location("causticsSheet"), 0);
        glUniform1i(causticsShader.location("terrainTexture"), 1);
        glUniform3f(causticsShader.location("sunDir"), sunDir.x, sunDir.y, sunDir.z);

        for (auto isl : earth.islands)
        {
            glUniformMatrix4fv(glGetUniformLocation(causticsShader.id(), "viewTrans"), 1, GL_FALSE, &cam.combined[0][0]);
            isl->terrainMesh->render();
        }

        // render waves to alpha channel of underwaterBuffer
        waveRenderer->render(newDeltaTime, cam.combined);

        underwaterBuffer.unbindCurrent();
        // DONE RENDERING UNDERWATER

        // RENDER ISLANDS:
        terrainShader.use();
        terrainTextures->bind(0);
        glDisable(GL_BLEND);
        glUniform1i(terrainShader.location("terrainTextures"), 0);
        glUniform3f(terrainShader.location("sunDir"), sunDir.x, sunDir.y, sunDir.z);
        glUniform1i(terrainShader.location("backgroundTerrainLayer"), 0);
        glUniform4f(terrainShader.location("terrainLayers"), 2, 3, 4, 5);
        glUniform4f(terrainShader.location("hasNormal"), 0, 0, 0, 0); // (background must have normal)

        for (auto isl : earth.islands)
        {
            if (isl == hoveredIsland) continue;

            glUniformMatrix4fv(terrainShader.location("viewTrans"), 1, GL_FALSE, &cam.combined[0][0]);
            isl->terrainMesh->render();
        }
        // DONE RENDERING ISLANDS

        // RENDER WATER:
        earthShader.use();
        glEnable(GL_BLEND);
        seaNormalMap->bind(0);
        seaDUDV->bind(1);
        underwaterBuffer.colorTexture->bind(2);
        underwaterBuffer.depthTexture->bind(3);
        foamTexture->bind(4);
        glm::mat4 mvp = cam.combined;
    
        glUniformMatrix4fv(glGetUniformLocation(earthShader.id(), "MVP"), 1, GL_FALSE, &mvp[0][0]);
        glUniform1i(earthShader.location("seaNormals"), 0);
        glUniform1i(earthShader.location("seaDUDV"), 1);
        glUniform1i(earthShader.location("underwaterTexture"), 2);
        glUniform1i(earthShader.location("underwaterDepthTexture"), 3);
        glUniform1i(earthShader.location("foamTexture"), 4);
        glUniform1f(earthShader.location("time"), time);
        glUniform2f(earthShader.location("scrSize"), gu::widthPixels, gu::heightPixels);
        glUniform3f(earthShader.location("camPos"), cam.position.x, cam.position.y, cam.position.z);
        glUniform3f(earthShader.location("sunDir"), sunDir.x, sunDir.y, sunDir.z);
        earth.mesh->render();
        // DONE RENDERING WATER

        // RENDER ATMOSPHERE:
        atmosphereShader.use();
        glDepthMask(false);

        mvp = glm::mat4(1.0f);
        glm::vec3 cP = glm::normalize(cam.position) * earth.sphere.radius;
        float lon = earth.longitude(cP.x, cP.z), lat = earth.latitude(cP.y);
        mvp = glm::rotate(mvp, -(lon + 90) * mu::DEGREES_TO_RAD, mu::Y);
        mvp = glm::rotate(mvp, lat * mu::DEGREES_TO_RAD, mu::X);
        mvp = cam.combined * mvp;

        glm::vec3 weirdSunDir = sunDir;
        weirdSunDir = glm::rotate(weirdSunDir, (lon + 90) * mu::DEGREES_TO_RAD, mu::Y);
        weirdSunDir = glm::rotate(weirdSunDir, -lat * mu::DEGREES_TO_RAD, mu::X);

        glUniformMatrix4fv(atmosphereShader.location("MVP"), 1, GL_FALSE, &mvp[0][0]);
        glUniform3f(atmosphereShader.location("sunDir"), weirdSunDir.x, weirdSunDir.y, weirdSunDir.z);
        glUniform1f(atmosphereShader.location("camDist"), glm::length(cam.position) - ATMOSPHERE_RADIUS);
        
        atmosphereMesh->render();
        glDepthMask(true);
        // DONE RENDERING ATMOSPHERE

        glDisable(GL_BLEND);

        lineRenderer.projection = cam.combined;
        lineRenderer.line(glm::vec3(-300, 0, 0), glm::vec3(300, 0, 0), mu::X);
        lineRenderer.line(glm::vec3(0, -300, 0), glm::vec3(0, 300, 0), mu::Y);
        lineRenderer.line(glm::vec3(0, 0, -300), glm::vec3(0, 0, 300), mu::Z);

        for (int i = 0; i < 100; i += 2)
            lineRenderer.line(sunDir * glm::vec3(i * 5), sunDir * glm::vec3((i + 1) * 5), glm::vec3(1, 1, 0));
    }

    void onResize()
    {
        cam.viewportWidth = gu::widthPixels;
        cam.viewportHeight = gu::heightPixels;
    }

    ~LevelScreen()
    {
        delete waveRenderer;
    }

};
