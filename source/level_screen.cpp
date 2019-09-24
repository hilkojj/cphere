
#include "gu/screen.h"
#include "gl_includes.h"
#include "graphics/texture.h"
#include "graphics/texture_array.h"
#include "level/planet.h"
#include "level/wave_renderer.h"
#include "level/space_renderer.h"
#include "level/cloud_renderer.h"
#include "level/planet_camera_movement.h"
#include "input/key_input.h"
#include "input/mouse_input.h"
#include "utils/camera/flying_camera_controller.h"
#include "graphics/shader_program.h"
#include "gu/game_utils.h"
#include "graphics/3d/debug_line_renderer.h"
#include "utils/math_utils.h"
#include "graphics/frame_buffer.h"
#include "utils/math/sphere_mesh_generator.h"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include "sea_graph.h"

#include "server_client/server.h"

#include "files/file.h"

#include <fstream>

const float EARTH_RADIUS = 150, ATMOSPHERE_RADIUS = 200;

class LevelScreen : public Screen
{

  public:
    Server *svr;

    // Planet earth;
    ShaderProgram earthShader, causticsShader, terrainShader, atmosphereShader, postProcessingShader;

    PerspectiveCamera cam;
    FlyingCameraController camController;
    PlanetCameraMovement planetCamMovement;

    bool camPlanetMode = true;

    DebugLineRenderer lineRenderer;
    SharedTexture seaNormalMap, seaDUDV, caustics, sand, foamTexture, seaWaves;
    SharedTexArray terrainTextures;
    SharedMesh atmosphereMesh;

    FrameBuffer underwaterBuffer, *sceneBuffer = NULL;
    WaveRenderer *waveRenderer;
    SpaceRenderer spaceRenderer;
    CloudRenderer cloudRenderer;

    SeaGraph seaGraph;

    float time = 0;

    LevelScreen(Server *svr)
        : 
          svr(svr),
          cam(PerspectiveCamera(.1, 1000, 1, 1, 55)), camController(&cam), planetCamMovement(&cam, &svr->level.earth),
          
          caustics(Texture::fromDDSFile("assets/textures/tc_caustics.dds")),
          sand(Texture::fromDDSFile("assets/textures/tc_sand.dds")),
          foamTexture(Texture::fromDDSFile("assets/textures/tc_foam.dds")),

          seaWaves(Texture::fromDDSFile("assets/textures/sea_waves.dds")),

          terrainTextures(TextureArray::fromDDSFiles({
              "assets/textures/tc_sand.dds",
              "assets/textures/tc_sand_normal.dds",

              "assets/textures/tc_grass.dds",
              "assets/textures/tc_grass_dead.dds",
          })),

          seaGraph(&svr->level.earth),

          earthShader(ShaderProgram::fromFiles("EarthShader", "assets/shaders/earth.vert", "assets/shaders/earth.frag")),
          atmosphereShader(ShaderProgram::fromFiles("EarthAtmosphereShader", "assets/shaders/earth_atmosphere.vert", "assets/shaders/earth_atmosphere.frag")),
          causticsShader(ShaderProgram::fromFiles("CausticsShader", "assets/shaders/terrain_caustics.vert", "assets/shaders/terrain_caustics.frag")),
          terrainShader(ShaderProgram::fromFiles("TerrainShader", "assets/shaders/terrain.vert", "assets/shaders/terrain.frag")),
          postProcessingShader(ShaderProgram::fromFiles("PostProcessingShader", "assets/shaders/post_processing.vert", "assets/shaders/post_processing.frag")),

          atmosphereMesh(SphereMeshGenerator::generate("earth_atmosphere", ATMOSPHERE_RADIUS, 50, 130, VertAttributes().add_(VertAttributes::POSITION).add_(VertAttributes::NORMAL))),
          cloudRenderer(&svr->level.earth),

          underwaterBuffer(FrameBuffer(1024, 1024))
    {
        underwaterBuffer.addColorTexture(GL_RGBA, GL_LINEAR, GL_LINEAR);
        underwaterBuffer.addDepthTexture(GL_LINEAR, GL_LINEAR);

        VertBuffer::uploadSingleMesh(atmosphereMesh);

        waveRenderer = new WaveRenderer(svr->level.earth);
        cam.position = glm::vec3(0, 0, 300);
        cam.lookAt(glm::vec3(0));
        cam.update();
        camController.speedMultiplier = 100;
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        MouseInput::setLockedMode(!camPlanetMode);
    }

    void render(double deltaTime)
    {
        double newDeltaTime =  deltaTime * (KeyInput::pressed(GLFW_KEY_KP_ADD) ? 15 : (KeyInput::pressed(GLFW_KEY_KP_SUBTRACT) ? 0 : 1));
        time += newDeltaTime;

        svr->update(newDeltaTime);
        svr->wait();
        svr->updateClients(newDeltaTime);
        svr->wait();
        // if (KeyInput::justPressed(GLFW_KEY_R))
        // {
        //     svr->level.earth.destroyIslands();
        //     svr->level.earth = Planet("earth", Sphere(EARTH_RADIUS));
        //     generateEarth(&svr->level.earth);
        //     delete waveRenderer;
        //     waveRenderer = new WaveRenderer(svr->level.earth);

        //     std::vector<uint8> data;
        //     svr->level.earth.toBinary(data);
        //     File::writeBinary("level.save", data);
        // }
        ShaderProgram::reloadFromFile = int(time * 2.) % 2 == 0;//KeyInput::justPressed(GLFW_KEY_F5);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Island *hoveredIsland = NULL;
        ivec2 hoveredTile(0);

        if (KeyInput::justPressed(GLFW_KEY_G))
            MouseInput::setLockedMode(false);
        else if (KeyInput::justReleased(GLFW_KEY_G) && !camPlanetMode)
            MouseInput::setLockedMode(true);

        if (KeyInput::justPressed(GLFW_KEY_C))
        {
            camPlanetMode = !camPlanetMode;
            MouseInput::setLockedMode(!camPlanetMode);
        }

        if (camPlanetMode)
            planetCamMovement.update(deltaTime); // planet camera movement
        else if (!KeyInput::pressed(GLFW_KEY_G))
            camController.update(deltaTime); // flying camera movement

        if (KeyInput::pressed(GLFW_KEY_G))
        {
            hoveredIsland = svr->level.earth.islUnderCursor(cam);
            if (hoveredIsland) hoveredIsland->tileUnderCursor(hoveredTile, cam);
        }

        glm::vec3 sunDir = glm::vec3(glm::sin(time * .008), 0, glm::cos(time * .008));

        // RENDER UNDERWATER:
        underwaterBuffer.bind();
        glEnable(GL_BLEND);

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        causticsShader.use();
        caustics->bind(0, causticsShader, "causticsSheet");
        sand->bind(1, causticsShader, "terrainTexture");

        glUniform1f(causticsShader.location("time"), time);
        glUniform3f(causticsShader.location("sunDir"), sunDir.x, sunDir.y, sunDir.z);

        for (auto isl : svr->level.earth.islands)
        {
            glUniformMatrix4fv(glGetUniformLocation(causticsShader.id(), "viewTrans"), 1, GL_FALSE, &cam.combined[0][0]);
            isl->terrainMesh->render();
        }

        // render waves to alpha channel of underwaterBuffer
        waveRenderer->render(newDeltaTime, cam.combined);

        underwaterBuffer.unbind();
        // DONE RENDERING UNDERWATER

        sceneBuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // RENDER ISLANDS:
        terrainShader.use();
        terrainTextures->bind(0);
        glDisable(GL_BLEND);
        glUniform1i(terrainShader.location("terrainTextures"), 0);
        glUniform3f(terrainShader.location("sunDir"), sunDir.x, sunDir.y, sunDir.z);
        glUniform1i(terrainShader.location("backgroundTerrainLayer"), 0);
        glUniform4f(terrainShader.location("terrainLayers"), 2, 3, 4, 5);
        glUniform4f(terrainShader.location("hasNormal"), 0, 0, 0, 0); // (background must have normal)

        for (auto isl : svr->level.earth.islands)
        {
            glUniformMatrix4fv(terrainShader.location("viewTrans"), 1, GL_FALSE, &(cam.combined[0][0]));
            isl->terrainMesh->mode = isl == hoveredIsland ? GL_LINES : GL_TRIANGLES;
            isl->terrainMesh->render();
            isl->terrainMesh->mode = GL_TRIANGLES;
        }
        // DONE RENDERING ISLANDS

        // RENDER WATER:
        earthShader.use();
        glEnable(GL_BLEND);
        foamTexture->bind(0, earthShader, "foamTexture");
        seaWaves->bind(1, earthShader, "seaWaves");
        underwaterBuffer.colorTexture->bind(2, earthShader, "underwaterTexture");
        underwaterBuffer.depthTexture->bind(3, earthShader, "underwaterDepthTexture");
        glm::mat4 mvp = cam.combined;
    
        glUniformMatrix4fv(earthShader.location("MVP"), 1, GL_FALSE, &mvp[0][0]);
        glUniform1f(earthShader.location("time"), time);
        glUniform2f(earthShader.location("scrSize"), gu::widthPixels, gu::heightPixels);
        glUniform3f(earthShader.location("camPos"), cam.position.x, cam.position.y, cam.position.z);
        glUniform3f(earthShader.location("sunDir"), sunDir.x, sunDir.y, sunDir.z);
        svr->level.earth.mesh->render();
        // DONE RENDERING WATER

        spaceRenderer.renderBox(sunDir, cam);

        // RENDER ATMOSPHERE:
        atmosphereShader.use();
        glDepthMask(false);
        glEnable(GL_BLEND);

        mvp = glm::mat4(1.0f);
        glm::vec3 cP = glm::normalize(cam.position) * svr->level.earth.sphere.radius;
        float lon = svr->level.earth.longitude(cP.x, cP.z), lat = svr->level.earth.latitude(cP.y);
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

        cloudRenderer.render(time, newDeltaTime, cam, sunDir);

        glDisable(GL_BLEND);

        lineRenderer.projection = cam.combined;
        lineRenderer.line(glm::vec3(-300, 0, 0), glm::vec3(300, 0, 0), mu::X);
        lineRenderer.line(glm::vec3(0, -300, 0), glm::vec3(0, 300, 0), mu::Y);
        lineRenderer.line(glm::vec3(0, 0, -300), glm::vec3(0, 0, 300), mu::Z);

        for (int i = 0; i < 100; i += 2)
            lineRenderer.line(sunDir * glm::vec3(i * 5), sunDir * glm::vec3((i + 1) * 5), glm::vec3(1, 1, 0));

        if (hoveredIsland)
        {
            lineRenderer.line(
                hoveredIsland->vertexPositionsPlanet[hoveredIsland->xyToVertI(hoveredTile.x, hoveredTile.y)],
                hoveredIsland->vertexPositionsPlanet[hoveredIsland->xyToVertI(hoveredTile.x, hoveredTile.y)] + hoveredIsland->vertexNormalsPlanet[hoveredIsland->xyToVertI(hoveredTile.x, hoveredTile.y)] * vec3(10),
                mu::X
            );
            lineRenderer.line(
                hoveredIsland->vertexPositionsPlanet[hoveredIsland->xyToVertI(hoveredTile.x + 1, hoveredTile.y)],
                hoveredIsland->vertexPositionsPlanet[hoveredIsland->xyToVertI(hoveredTile.x + 1, hoveredTile.y)] + hoveredIsland->vertexNormalsPlanet[hoveredIsland->xyToVertI(hoveredTile.x, hoveredTile.y)] * vec3(10),
                mu::X
            );

            lineRenderer.line(
                hoveredIsland->vertexPositionsPlanet[hoveredIsland->xyToVertI(hoveredTile.x, hoveredTile.y + 1)],
                hoveredIsland->vertexPositionsPlanet[hoveredIsland->xyToVertI(hoveredTile.x, hoveredTile.y + 1)] + hoveredIsland->vertexNormalsPlanet[hoveredIsland->xyToVertI(hoveredTile.x, hoveredTile.y)] * vec3(10),
                mu::X
            );

            lineRenderer.line(
                hoveredIsland->vertexPositionsPlanet[hoveredIsland->xyToVertI(hoveredTile.x + 1, hoveredTile.y + 1)],
                hoveredIsland->vertexPositionsPlanet[hoveredIsland->xyToVertI(hoveredTile.x + 1, hoveredTile.y + 1)] + hoveredIsland->vertexNormalsPlanet[hoveredIsland->xyToVertI(hoveredTile.x, hoveredTile.y)] * vec3(10),
                mu::X
            );
        }

        vec2 mouseLonLat(0);
        svr->level.earth.cursorToLonLat(&cam, mouseLonLat);

        std::cout << to_string(mouseLonLat) << "\n";

        Node nearestToMouse = seaGraph.nearest(mouseLonLat);

        for (auto &n : seaGraph.nodes)
        {
            for (auto &n2 : n->connections)
            {
                lineRenderer.line(
                    n->position * float(1.01), n2->position * float(1.01), vec3(1, vec2(n->distToCoast / 5.))
                );

                // std::cout << to_string(n->position) << " -> " << to_string(n2->position) << "\n";
            }
            if (n == nearestToMouse)
                lineRenderer.line(n->position, n->position * float(1.1), mu::Z);
        }

        sceneBuffer->unbind();
        glEnable(GL_BLEND);

        postProcessingShader.use();
        glUniform1f(postProcessingShader.location("zoomEffect"), planetCamMovement.zoomVelocity / 2.);
        glUniform2f(postProcessingShader.location("resolution"), gu::widthPixels, gu::heightPixels);
        sceneBuffer->colorTexture->bind(0, postProcessingShader, "scene");
        glDisable(GL_DEPTH_TEST);
        Mesh::getQuad()->render();
        spaceRenderer.renderSun(sunDir, cam, sceneBuffer->depthTexture, time, svr->level.earth);
        glEnable(GL_DEPTH_TEST);
    }

    void onResize()
    {
        cam.viewportWidth = gu::widthPixels;
        cam.viewportHeight = gu::heightPixels;
        if (sceneBuffer) delete sceneBuffer;
        sceneBuffer = new FrameBuffer(gu::widthPixels, gu::heightPixels, 4);
        sceneBuffer->addColorTexture(GL_RGB, GL_LINEAR, GL_LINEAR);
        sceneBuffer->addDepthTexture(GL_LINEAR, GL_LINEAR);
    }

    ~LevelScreen()
    {
        delete waveRenderer;
    }

};
