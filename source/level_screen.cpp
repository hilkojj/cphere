
#include "gu/screen.h"
#include "gl_includes.h"
#include "graphics/texture.h"
#include "graphics/texture_array.h"
#include "level/planet.h"
#include "level/graphics/wave_renderer.h"
#include "level/graphics/space_renderer.h"
#include "level/graphics/cloud_renderer.h"
#include "level/graphics/planet_camera_movement.h"
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
#include "utils/json_model_loader.h"
#include "level/graphics/ship_wake.h"
#include "level/systems/building_rendering_system.h"

#include "level/level.h"

#include <fstream>

class LevelScreen : public Screen
{
  public:
    Level *lvl;

    ShaderProgram earthShader, causticsShader, terrainShader, atmosphereShader, postProcessingShader, shipWakeShader, shipShader;

    PerspectiveCamera cam;
    FlyingCameraController camController;
    PlanetCameraMovement planetCamMovement;

    bool camPlanetMode = true;

    DebugLineRenderer lineRenderer;
    SharedTexture caustics, sand, foamTexture, seaWaves, shipTexture;
    SharedTexArray terrainTextures;
    SharedMesh atmosphereMesh, shipMesh;

    ShipWake shipWake;

    FrameBuffer underwaterBuffer, reflectionBuffer, *sceneBuffer = NULL;
    WaveRenderer *waveRenderer;
    SpaceRenderer spaceRenderer;
    CloudRenderer cloudRenderer;

    LevelScreen(const char *loadFilePath)
        :
          lvl(new Level(loadFilePath)),

          cam(PerspectiveCamera(.1, 1000, 1, 1, 55)), camController(&cam), planetCamMovement(&cam, &lvl->earth),
          
          caustics(Texture::fromDDSFile("assets/textures/tc_caustics.dds")),
          sand(Texture::fromDDSFile("assets/textures/tc_sand.dds")),
          foamTexture(Texture::fromDDSFile("assets/textures/tc_foam.dds")),
          shipTexture(Texture::fromDDSFile("assets/textures/cogship.dds")),

          seaWaves(Texture::fromDDSFile("assets/textures/sea_waves.dds")),

          terrainTextures(TextureArray::fromDDSFiles({
              "assets/textures/tc_sand.dds",
              "assets/textures/tc_sand_normal.dds",

              "assets/textures/tc_rock.dds",
              "assets/textures/tc_rock_normal.dds",

              "assets/textures/tc_grass.dds",

              "assets/textures/tc_grass_dead.dds",

              "assets/textures/tc_rock2.dds",
              "assets/textures/tc_rock2_normal.dds",
          })),

          earthShader(ShaderProgram::fromFiles("EarthShader", "assets/shaders/earth.vert", "assets/shaders/earth.frag")),
          atmosphereShader(ShaderProgram::fromFiles("EarthAtmosphereShader", "assets/shaders/earth_atmosphere.vert", "assets/shaders/earth_atmosphere.frag")),
          causticsShader(ShaderProgram::fromFiles("CausticsShader", "assets/shaders/terrain_caustics.vert", "assets/shaders/terrain_caustics.frag")),
          terrainShader(ShaderProgram::fromFiles("TerrainShader", "assets/shaders/terrain.vert", "assets/shaders/terrain.frag")),
          postProcessingShader(ShaderProgram::fromFiles("PostProcessingShader", "assets/shaders/post_processing.vert", "assets/shaders/post_processing.frag")),
          shipWakeShader(ShaderProgram::fromFiles("ShipWakeShader", "assets/shaders/ship_wake.vert", "assets/shaders/ship_wake.frag")),
          shipShader(ShaderProgram::fromFiles("ShipShader", "assets/shaders/ship.vert", "assets/shaders/ship.frag")),

          atmosphereMesh(SphereMeshGenerator::generate("earth_atmosphere", ATMOSPHERE_RADIUS, 50, 130, VertAttributes().add_(VertAttributes::POSITION).add_(VertAttributes::NORMAL))),
          cloudRenderer(&lvl->earth),

          underwaterBuffer(FrameBuffer(1024, 1024)),
          reflectionBuffer(FrameBuffer(512, 512))
    {
        underwaterBuffer.addColorTexture(GL_RGBA, GL_LINEAR, GL_LINEAR);
        underwaterBuffer.addDepthTexture(GL_LINEAR, GL_LINEAR);

        reflectionBuffer.addColorTexture(GL_RGB, GL_LINEAR, GL_LINEAR);
        reflectionBuffer.addDepthBuffer();

        VertBuffer::uploadSingleMesh(atmosphereMesh);

        waveRenderer = new WaveRenderer(lvl->earth);
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

        auto posTexNorAttrs = VertAttributes().add_(VertAttributes::POSITION).add_(VertAttributes::TEX_COORDS).add_(VertAttributes::NORMAL);

        {
            SharedModel model = JsonModelLoader::fromUbjsonFile("assets/models/cogship.ubj", &posTexNorAttrs)[0];
            shipMesh = model->parts[0].mesh;
            VertBuffer::uploadSingleMesh(shipMesh);
        }
        MouseInput::setLockedMode(!camPlanetMode);
    }

    void render(double deltaTime)
    {
        if (KeyInput::justPressed(GLFW_KEY_R))
        {
            delete lvl;
            lvl = new Level();
            delete waveRenderer;
            waveRenderer = new WaveRenderer(lvl->earth);
        }
        lvl->lineRenderer = &lineRenderer;
        lvl->cam = &cam;

        double newDeltaTime =  deltaTime * (KeyInput::pressed(GLFW_KEY_KP_ADD) ? 15 : (KeyInput::pressed(GLFW_KEY_KP_SUBTRACT) ? .1 : 1));
        
        auto &time = lvl->time;
        auto &earth = lvl->earth;

        #ifndef EMSCRIPTEN
        ShaderProgram::reloadFromFile = int(time * 2.) % 2 == 0;//KeyInput::justPressed(GLFW_KEY_F5);
        #endif

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
            planetCamMovement.update(deltaTime, &lvl->earth); // planet camera movement
        else if (!KeyInput::pressed(GLFW_KEY_G))
            camController.update(deltaTime); // flying camera movement

        if (KeyInput::pressed(GLFW_KEY_G))
        {
            hoveredIsland = earth.islUnderCursor(&cam);
            if (hoveredIsland) hoveredIsland->tileUnderCursor(hoveredTile, &cam);
        }

        glm::vec3 sunDirZoomedOut = glm::vec3(glm::sin(time * .008), 0, glm::cos(time * .008));
        glm::vec3 sunDirZoomedIn = rotate(mu::Z, (planetCamMovement.lat - float(90.)) * mu::DEGREES_TO_RAD, mu::X);
        sunDirZoomedIn = rotate(sunDirZoomedIn, (planetCamMovement.lon + float(6.)) * mu::DEGREES_TO_RAD, mu::Y);
        float closeSunDir = clamp((planetCamMovement.actualZoom - .35) * 3., 0., .7);
        glm::vec3 sunDir = sunDirZoomedOut * (1 - closeSunDir) + sunDirZoomedIn * closeSunDir;
        sunDir = normalize(sunDir);

        vec2 ll;
        earth.cursorToLonLat(&cam, ll);
        vec3 shipPos;// earth.lonLatTo3d(ll.x, ll.y, 0);

        // RENDER WATER REFLECTIONS
        reflectionBuffer.bind();
        glClearColor(.12, .15, .29, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float prevCamFar = cam.far_;
        cam.far_ = planetCamMovement.horizonDistance - 10.;
        cam.update();

        {
            shipShader.use();
            glUniform1i(shipShader.location("reflection"), 1);
            shipTexture->bind(0, shipShader, "shipTexture");
            glCullFace(GL_FRONT);

            for (auto &ship : lvl->ships) {
                auto worldTrans = ship.transform;
                vec3 localSunDir = vec4(sunDir, 1) * worldTrans;
                glUniform3f(shipShader.location("sunDir"), localSunDir.x, localSunDir.y, localSunDir.z);
                glUniformMatrix4fv(shipShader.location("mvp"), 1, GL_FALSE, &(cam.combined * worldTrans)[0][0]);
                shipMesh->render();

                shipPos = earth.lonLatTo3d(ship.lonLat.x, ship.lonLat.y, 0);
            };
            glCullFace(GL_BACK);
        }

        reflectionBuffer.unbind();
        cam.far_ = prevCamFar;
        cam.update();

        // DONE RENDERING WATER REFLECTIONS

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

        for (auto isl : earth.islands)
        {
            glUniformMatrix4fv(glGetUniformLocation(causticsShader.id(), "viewTrans"), 1, GL_FALSE, &cam.combined[0][0]);
            isl->terrainMesh->render();
        }
        //         shipShader.use();
        // glUniformMatrix4fv(shipShader.location("mvp"), 1, GL_FALSE, &(rotate(rotate(translate(cam.combined, vec3(0, 0, EARTH_RADIUS - sin(time) * 3. - 1.5)), 90 * mu::DEGREES_TO_RAD, mu::X), 130 * mu::DEGREES_TO_RAD, mu::Y)[0][0]));
        // shipTexture->bind(0, shipShader, "shipTexture");
        // ship->render();

        // render waves to alpha channel of underwaterBuffer
        waveRenderer->render(newDeltaTime, cam.combined);

        shipWakeShader.use();
        glUniformMatrix4fv(shipWakeShader.location("viewTrans"), 1, GL_FALSE, &(cam.combined[0][0]));
        shipWake.render(lineRenderer, shipPos, newDeltaTime);

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
        glUniform4f(terrainShader.location("terrainLayers"), 2, 4, 5, 6);
        glUniform4i(terrainShader.location("hasNormal"), 1, 0, 0, 1); // (background must have normal)
        glUniform4i(terrainShader.location("fadeBlend"), 1, 0, 0, 1);
        glUniform4f(terrainShader.location("specularity"), .4, 0, 0, .6);
        glUniform4f(terrainShader.location("textureScale"), 2.2, 1., 1., 1.5);

        glUniformMatrix4fv(terrainShader.location("viewTrans"), 1, GL_FALSE, &(cam.combined[0][0]));
        for (auto isl : earth.islands)
        {
            isl->terrainMesh->mode = isl == hoveredIsland ? GL_LINES : GL_TRIANGLES;
            isl->terrainMesh->render();
            isl->terrainMesh->mode = GL_TRIANGLES;
        }
        // DONE RENDERING ISLANDS

        BuildingRenderingSystem::active->render(newDeltaTime, lvl);

        shipShader.use();
        glUniform1i(shipShader.location("reflection"), 0);
        shipTexture->bind(0, shipShader, "shipTexture");

        for (auto &ship : lvl->ships) {
            auto &worldTrans = ship.transform;
            vec3 localSunDir = vec4(sunDir, 1) * worldTrans;
            glUniform3f(shipShader.location("sunDir"), localSunDir.x, localSunDir.y, localSunDir.z);
            glUniformMatrix4fv(shipShader.location("mvp"), 1, GL_FALSE, &(cam.combined * worldTrans)[0][0]);
            shipMesh->render();
        };

        // RENDER WATER:
        earthShader.use();
        glEnable(GL_BLEND);
        foamTexture->bind(0, earthShader, "foamTexture");
        seaWaves->bind(1, earthShader, "seaWaves");
        underwaterBuffer.colorTexture->bind(2, earthShader, "underwaterTexture");
        underwaterBuffer.depthTexture->bind(3, earthShader, "underwaterDepthTexture");
        reflectionBuffer.colorTexture->bind(4, earthShader, "reflectionTexture");
        glm::mat4 mvp = cam.combined;
    
        glUniformMatrix4fv(earthShader.location("MVP"), 1, GL_FALSE, &mvp[0][0]);
        glUniform1f(earthShader.location("time"), time);
        glUniform2f(earthShader.location("scrSize"), gu::widthPixels, gu::heightPixels);
        glUniform3f(earthShader.location("camPos"), cam.position.x, cam.position.y, cam.position.z);
        glUniform3f(earthShader.location("sunDir"), sunDir.x, sunDir.y, sunDir.z);
        earth.mesh->render();
        // DONE RENDERING WATER

        spaceRenderer.renderBox(sunDirZoomedOut, cam, planetCamMovement.actualZoom);

        // RENDER ATMOSPHERE:
        atmosphereShader.use();
        glDepthMask(false);
        glEnable(GL_BLEND);

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

        cloudRenderer.render(time, newDeltaTime, cam, sunDir, &lvl->earth);

        glDisable(GL_BLEND);

        lineRenderer.projection = cam.combined;
        // lineRenderer.line(glm::vec3(-300, 0, 0), glm::vec3(300, 0, 0), mu::X);
        // lineRenderer.line(glm::vec3(0, -300, 0), glm::vec3(0, 300, 0), mu::Y);
        // lineRenderer.line(glm::vec3(0, 0, -300), glm::vec3(0, 0, 300), mu::Z);

        // lineRenderer.line(shipPos, shipPos + vec3(0, 10, 0), mu::X);
        lvl->update(newDeltaTime);

        for (int i = 0; i < 100; i += 2)
            lineRenderer.line(sunDir * glm::vec3(i * 5), sunDir * glm::vec3((i + 1) * 5), glm::vec3(1, 1, 0));

        if (hoveredIsland)
        {
            // std::cout << hoveredIsland->percentageUnderwater() << "\n";
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

        for (Island *isl : lvl->earth.islands)
        {
            for (int x = 0; x < isl->width; x++)
            {
                for (int y = 0; y < isl->height; y++)
                {
                    auto b = isl->getBuilding(x, y);
                    if (b)
                    {
                        vec3 n = isl->vertexNormalsPlanet[isl->xyToVertI(x, y)] * float (.1);
                        lineRenderer.line(
                                isl->vertexPositionsPlanet[isl->xyToVertI(x, y)] + n,
                                isl->vertexPositionsPlanet[isl->xyToVertI(x + 1, y + 1)] + n,
                                mu::X
                        );
                        lineRenderer.line(
                                isl->vertexPositionsPlanet[isl->xyToVertI(x + 1, y)] + n,
                                isl->vertexPositionsPlanet[isl->xyToVertI(x, y + 1)] + n,
                                mu::X
                        );
                    }
                }
            }
        }

        vec2 mouseLonLat(0);
        bool mouseOnEarth = earth.cursorToLonLat(&cam, mouseLonLat);

        Node nearestToMouse = lvl->seaGraph.nearest(mouseLonLat);

        std::vector<WayPoint> path;
        glLineWidth(3.);

        for (auto &ship : lvl->ships) {
            if (!ship.path) continue;
            auto &path = ship.path->points;
            vec3 prev = path[0].position;
            for (auto &n : path)
            {
                lineRenderer.line(prev * float(1.005), n.position * float(1.005), mu::Y);
                prev = n.position;
            }
        };
        
        if (mouseOnEarth && KeyInput::pressed(GLFW_KEY_P) && lvl->seaGraph.findPath(vec2(0, 0), mouseLonLat, path))
        {
            vec3 prev = path[0].position;
            for (auto &n : path)
            {
                lineRenderer.line(prev * float(1.005), n.position * float(1.005), mu::Y);
                prev = n.position;
            }
        }
        glLineWidth(1.);

        // for (auto &n : seaGraph.nodes)
        // {
        //     // for (auto &n2 : n->connections)
        //     // {
        //     //     lineRenderer.line(
        //     //         n->position * float(1.01), n2->position * float(1.01), vec3(1 - n->distToCoast / 5., .3, .8)
        //     //     );

        //     //     // std::cout << to_string(n->position) << " -> " << to_string(n2->position) << "\n";
        //     // }
        //     if (n == nearestToMouse)
        //         lineRenderer.line(n->position, n->position * float(1.1), mu::Z);
        // }

        sceneBuffer->unbind();
        glEnable(GL_BLEND);

        postProcessingShader.use();
        glUniform1f(postProcessingShader.location("zoomEffect"), planetCamMovement.zoomVelocity / 2.);
        glUniform1f(postProcessingShader.location("zoom"), planetCamMovement.actualZoom);
        glUniform2f(postProcessingShader.location("resolution"), gu::widthPixels, gu::heightPixels);
        sceneBuffer->colorTexture->bind(0, postProcessingShader, "scene");
        sceneBuffer->depthTexture->bind(1, postProcessingShader, "sceneDepth");
        glDisable(GL_DEPTH_TEST);
        Mesh::getQuad()->render();
        spaceRenderer.renderSun(sunDir, cam, sceneBuffer->depthTexture, time, earth);
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
        delete lvl;
    }
};
