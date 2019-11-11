
#include "gu/screen.h"
#include "gl_includes.h"
#include "graphics/texture.h"
#include "graphics/texture_array.h"
#include "level/planet.h"
#include "FastNoise.h"
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
#include "level/graphics/shadow_renderer.h"

#include "level/systems/building_rendering_system.h"
#include "level/level.h"
#include "planet_generation/island_generator.h"
#include "planet_generation/island_shape_generator.h"

#include <fstream>
#include <graphics/3d/perspective_camera.h>
#include <utils/math/interpolation.h>


namespace
{

    const float SEA_BOTTOM = -5, SEA_LEVEL = 0, LAND_LEVEL = 1.0f;
    const int ROCK_TEX = 0, GRASS_TEX = 1, DEAD_GRASS_TEX = 2, ROCK2_TEX = 3;

    void terrainFromShape2(std::vector<bool> shape, Island *isl)
    {
        FastNoise noise;
        for (int x = 0; x <= isl->width; x++)
        {
            for (int y = 0; y <= isl->height; y++)
            {
                int beachWidth = 20 + (int)(20 * abs(noise.GetNoise(x / 2, y / 2)));
                float distToSea = beachWidth;

                for (int x0 = max(0, x - beachWidth); x0 <= min(isl->width, x + beachWidth); x0++)
                {
                    for (int y0 = max(0, y - beachWidth); y0 <= min(isl->height, y + beachWidth); y0++)
                    {
                        if (!shape[isl->xyToVertI(x0, y0)])
                        {
                            // is sea.
                            int xDiff = x - x0;
                            int yDiff = y - y0;
                            distToSea = min(distToSea, (float)sqrt(xDiff * xDiff + yDiff * yDiff));
                        }
                    }
                }
                float height = Interpolation::powOut(distToSea / beachWidth, 2);
                height = SEA_BOTTOM + height * abs(SEA_BOTTOM - LAND_LEVEL);
                isl->vertexPositionsOriginal[isl->xyToVertI(x, y)].y = height;
            }
        }
        FastNoise hillNoise;
        for (int x = 0; x <= isl->width; x++)
        {
            for (int y = 0; y <= isl->height; y++)
            {
                float hilliness = Interpolation::pow(isl->distToHeight(x, y, SEA_BOTTOM, SEA_LEVEL + .1, 20) / 20., 2);
                float hillHeight = clamp(hillNoise.GetSimplexFractal(x * 1.6, y * 1.6) * .5 + .2, 0., 1.);
                isl->vertexPositionsOriginal[isl->xyToVertI(x, y)].y += hilliness * hillHeight * 4.;
            }
        }
        FastNoise mountainNoise(isl->width * isl->height);
        for (int x = 0; x <= isl->width; x++)
        {
            for (int y = 0; y <= isl->height; y++)
            {
                float mountainNess = isl->distToHeight(x, y, SEA_BOTTOM, SEA_LEVEL + .1, 5) / 5.;
                mountainNess *= 1. - isl->distToHeight(x, y, SEA_BOTTOM, SEA_LEVEL + .1, 20) / 20.;

                mountainNess *= clamp(mountainNoise.GetSimplexFractal(x, y) * 10., 0., 1.);
                mountainNess *= clamp(mountainNoise.GetSimplexFractal(x + 200., y + 200.) * 10., 0., 1.);

                float mountainHeight = clamp(mountainNoise.GetSimplexFractal(x * 2.2, y * 2.2) * .5 + .25, 0., 1.);

                isl->vertexPositionsOriginal[isl->xyToVertI(x, y)].y += mountainNess * mountainHeight * 23.;
            }
        }
    }

    void generateIslandTerrain2(Island *isl)
    {
        isl->seaBottom = SEA_BOTTOM;
        terrainFromShape2(IslandShapeGenerator(isl).shape, isl);
    }

    void addGrass2(Island *isl)
    {
        FastNoise noise;
        for (int x = 0; x <= isl->width; x++)
        {
            for (int y = 0; y <= isl->height; y++)
            {
                float height = isl->vertexPositionsOriginal[isl->xyToVertI(x, y)].y;
                if (height < 0 || height > 7) continue;

                float rockness = clamp(isl->tileSteepness(x, y) - .01, 0., 1.);

                int maxDist = 1 + (int)(30 * abs(noise.GetNoise(x * 6, y * 6)));
                float dist = max(0.0f, isl->distToHeight(x, y, -100, 0, maxDist + 3) - 3);
                isl->textureMap[isl->xyToVertI(x, y)][GRASS_TEX] = clamp<float>(Interpolation::powOut(dist / maxDist, 2) - rockness, 0., 1.);
            }
        }
    }

    void addDeadGrass2(Island *isl)
    {
        FastNoise noise;
        noise.SetNoiseType(FastNoise::SimplexFractal);

        float noiseOffset = mu::random(1000);

        for (int x = 0; x <= isl->width; x++)
        {
            for (int y = 0; y <= isl->height; y++)
            {
                if (isl->textureMap[isl->xyToVertI(x, y)][GRASS_TEX] != 1) continue;

                float noiseX = x + noiseOffset, noiseY = y + noiseOffset;

                noiseX += noise.GetNoise(noiseX, noiseY) * 100;
                noiseY += noise.GetNoise(noiseY, noiseX) * 100;

                isl->textureMap[isl->xyToVertI(x, y)][DEAD_GRASS_TEX] = clamp((noise.GetNoise(noiseX, noiseY) - .2) * 3. + isl->textureMap[isl->xyToVertI(x, y)][DEAD_GRASS_TEX], 0.0, 1.0);
            }
        }
    }

    void addRockTexture2(Island *isl)
    {
        for (int x = 0; x <= isl->width; x++)
        {
            for (int y = 0; y <= isl->height; y++)
            {
                float height = isl->vertexPositionsOriginal[isl->xyToVertI(x, y)].y;
                float rockness = clamp<float>(isl->tileSteepness(x, y), 0., 1.) + (height > 3 ? 1 : 0);
                rockness *= 1. - isl->distToHeight(x, y, 1.5, 999, 3) / 3.;
                isl->textureMap[isl->xyToVertI(x, y)][ROCK_TEX] = rockness;
            }
        }
    }

    void addRock2Texture2(Island *isl)
    {
        for (int x = 0; x <= isl->width; x++)
        {
            for (int y = 0; y <= isl->height; y++)
            {
                float height = isl->vertexPositionsOriginal[isl->xyToVertI(x, y)].y + isl->tileSteepness(x, y);

                if (height < 9.) continue;

                float maxHeight = 0.;
                mu::spiral(4, [&](ivec2 p) {
                    if (p.x < 0 || p.y < 0 || p.x > isl->width || p.y > isl->height) return true;

                    maxHeight = max(maxHeight, isl->vertexPositionsOriginal[isl->xyToVertI(p.x, p.y)].y);
                    return true;
                });

                float snow = 1. - clamp<float>(maxHeight - height, 0., 1.);
                isl->textureMap[isl->xyToVertI(x, y)][ROCK2_TEX] = snow;
            }
        }
    }

    void islandTextureMapper2(Island *isl)
    {
        addGrass2(isl);
        addDeadGrass2(isl);
        addDeadGrass2(isl);
        addRockTexture2(isl);
        addRock2Texture2(isl);
    }

} // namespace

//SharedMesh earthMeshGenerator2(Planet *earth)
//{
//    VertAttributes attrs;
//    attrs.add_(VertAttributes::POSITION)
//            .add_(VertAttributes::NORMAL)
//            .add_(VertAttributes::TANGENT)
//            .add_(VertAttributes::TEX_COORDS);
//    return SphereMeshGenerator::generate(earth->name + "_mesh", earth->sphere.radius, 100, 70, attrs);
//}


class LevelScreen : public Screen
{
  public:
    Level *lvl;

    Island *isl;

    ShaderProgram earthShader, causticsShader, terrainShader, atmosphereShader, postProcessingShader, shipWakeShader, shipShader;

    PerspectiveCamera cam;
    FlyingCameraController camController;
    PlanetCameraMovement planetCamMovement;

    bool camPlanetMode = false;

    DebugLineRenderer lineRenderer;
    SharedTexture caustics, sand, foamTexture, seaWaves, shipTexture;
    SharedTexArray terrainTextures;
    SharedMesh atmosphereMesh, shipMesh;

    ShipWake shipWake;

    FrameBuffer underwaterBuffer, reflectionBuffer, *sceneBuffer = NULL;
    WaveRenderer *waveRenderer;
    SpaceRenderer spaceRenderer;
    CloudRenderer cloudRenderer;
    ShadowRenderer shadowRenderer;

    LevelScreen(const char *loadFilePath)
        :
          lvl(new Level(loadFilePath)),

          cam(.1, 1000, 1, 1, 55),
          camController(&cam), planetCamMovement(&cam, &lvl->earth),
          
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

          underwaterBuffer(1024, 1024),
          reflectionBuffer(512, 512)
    {
        underwaterBuffer.addColorTexture(GL_RGBA, GL_LINEAR, GL_LINEAR);
        underwaterBuffer.addDepthTexture(GL_LINEAR, GL_LINEAR);

        reflectionBuffer.addColorTexture(GL_RGB, GL_LINEAR, GL_LINEAR);
        reflectionBuffer.addDepthBuffer();

        VertBuffer::uploadSingleMesh(atmosphereMesh);

        waveRenderer = new WaveRenderer(lvl->earth);
        cam.position = glm::vec3(50, 50, 50);
        cam.lookAt(mu::Y * float(-30.));
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

//        IslandGenerator gen(132, 123, &lvl->earth, generateIslandTerrain2, islandTextureMapper2);
        IslandGenerator gen(110, 110, &lvl->earth, generateIslandTerrain2, islandTextureMapper2);
        isl = gen.generateEssentials();
        gen.finishGeneration();
//        isl.createMesh();
        VertBuffer::uploadSingleMesh(isl->terrainMesh);
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

        if (KeyInput::pressed(GLFW_KEY_SPACE)) lvl->time = 0;

//        if (camPlanetMode)
//            planetCamMovement.update(deltaTime, &lvl->earth); // planet camera movement
//        else if (!KeyInput::pressed(GLFW_KEY_G))
//            camController.update(deltaTime); // flying camera movement

        if (KeyInput::pressed(GLFW_KEY_G))
        {
            hoveredIsland = earth.islUnderCursor(&cam);
            if (hoveredIsland) hoveredIsland->tileUnderCursor(hoveredTile, &cam);
        }

        glm::vec3 sunDirZoomedOut = glm::vec3(glm::sin(time * .008), 0, glm::cos(time * .008));
        glm::vec3 sunDirZoomedIn = rotate(mu::Z, (planetCamMovement.lat - float(90.)) * mu::DEGREES_TO_RAD, mu::X);
        sunDirZoomedIn = rotate(sunDirZoomedIn, (planetCamMovement.lon + float(20.)) * mu::DEGREES_TO_RAD, mu::Y);
        float closeSunDir = clamp((planetCamMovement.actualZoom - .35) * 3., 0., .7);
        glm::vec3 sunDir = sunDirZoomedOut * (1 - closeSunDir) + sunDirZoomedIn * closeSunDir;
        sunDir = normalize(sunDir);

        vec2 ll;
        earth.cursorToLonLat(&cam, ll);

        shadowRenderer.begin(cam, -sunDir);

        BuildingRenderingSystem::active->renderShadows(lvl, &shadowRenderer.sunCam);
        glDisable(GL_BLEND);
        renderShips(sunDir, shadowRenderer.sunCam.combined);
        glEnable(GL_BLEND);

        shadowRenderer.end();


        // RENDER WATER REFLECTIONS
//        reflectionBuffer.bind();
//        glClearColor(.12, .15, .29, 1);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        float prevCamFar = cam.far_;
//        cam.far_ = planetCamMovement.horizonDistance - 10.;
//        cam.update();
//
//        renderShips(sunDir, cam.combined, true);
//
//        reflectionBuffer.unbind();
//        cam.far_ = prevCamFar;
//        cam.update();
//
//        // DONE RENDERING WATER REFLECTIONS
//
//        // RENDER UNDERWATER:
//        underwaterBuffer.bind();
//        glEnable(GL_BLEND);
//
//        glClearColor(0, 0, 0, 1);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//        causticsShader.use();
//        caustics->bind(0, causticsShader, "causticsSheet");
//        sand->bind(1, causticsShader, "terrainTexture");
//
//        glUniform1f(causticsShader.location("time"), time);
//        glUniform3f(causticsShader.location("sunDir"), sunDir.x, sunDir.y, sunDir.z);
//        glUniformMatrix4fv(glGetUniformLocation(causticsShader.id(), "viewTrans"), 1, GL_FALSE, &cam.combined[0][0]);
//
//        for (auto isl : earth.islands)
//        {
//            if (!isl->isInView) continue;
//            isl->terrainMesh->render();
//        }
//        // render waves to alpha channel of underwaterBuffer
//        cam.far_ = planetCamMovement.horizonDistance;
//        cam.update();
//        waveRenderer->render(newDeltaTime, cam.combined);
//
//        shipWakeShader.use();
//        glUniformMatrix4fv(shipWakeShader.location("viewTrans"), 1, GL_FALSE, &(cam.combined[0][0]));
//        shipWake.render(lineRenderer, lvl->ships[0].pos, newDeltaTime);
//        cam.far_ = prevCamFar;
//        cam.update();
//
//        underwaterBuffer.unbind();

        // DONE RENDERING UNDERWATER

        sceneBuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // RENDER ISLANDS:
        terrainShader.use();
        terrainTextures->bind(0);
        glDisable(GL_BLEND);
        glUniform1i(terrainShader.location("terrainTextures"), 0);
        glUniform1f(terrainShader.location("time"), time);
        glUniform3f(terrainShader.location("sunDir"), sunDir.x, sunDir.y, sunDir.z);
        glUniform1i(terrainShader.location("backgroundTerrainLayer"), 0);
        glUniform4f(terrainShader.location("terrainLayers"), 2, 4, 5, 6);
        glUniform4i(terrainShader.location("hasNormal"), 1, 0, 0, 1); // (background must have normal)
        glUniform4i(terrainShader.location("fadeBlend"), 0, 0, 0, 1);
        glUniform4f(terrainShader.location("specularity"), .4, 0, 0, .6);
        glUniform4f(terrainShader.location("textureScale"), 2.2, 1., 1., 1.5);

        shadowRenderer.sunDepthTexture->bind(1, terrainShader, "shadowBuffer");
        mat4 shadowMatrix = ShadowRenderer::BIAS_MATRIX * shadowRenderer.sunCam.combined;
        glUniformMatrix4fv(terrainShader.location("shadowMatrix"), 1, GL_FALSE, &((shadowMatrix)[0][0]));

        static bool b = false;
        if (KeyInput::justPressed(GLFW_KEY_Y)) b = !b;

        if (b)
            glUniformMatrix4fv(terrainShader.location("viewTrans"), 1, GL_FALSE, &(shadowRenderer.sunCam.combined[0][0]));
        else
            glUniformMatrix4fv(terrainShader.location("viewTrans"), 1, GL_FALSE, &(cam.combined[0][0]));


        if (b)
            lvl->cam = &shadowRenderer.sunCam;
        else
            lvl->cam = &cam;
        isl->terrainMesh->mode = time > 15 ? GL_TRIANGLES : GL_LINES;
        isl->terrainMesh->render();
//        for (auto isl : earth.islands)
//        {
//            if (!isl->isInView) continue;
//            isl->terrainMesh->mode = isl == hoveredIsland ? GL_LINES : GL_TRIANGLES;
//            isl->terrainMesh->render();
//            isl->terrainMesh->mode = GL_TRIANGLES;
//        }
        // DONE RENDERING ISLANDS

        /*

        renderShips(sunDir, cam.combined);

        glEnable(GL_BLEND);

        BuildingRenderingSystem::active->render(newDeltaTime, lvl, sunDir);

        if (BuildingsSystem::active->currentlyPlacing)
        {
            BuildingRenderingSystem::active->renderGhost(lvl, BuildingsSystem::active->currentlyPlacing, sunDir, BuildingsSystem::active->placingBlocked);
        }

        // RENDER WATER:
        earthShader.use();
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

        shadowRenderer.sunDepthTexture->bind(5, earthShader, "shadowBuffer");
        glUniformMatrix4fv(earthShader.location("shadowMatrix"), 1, GL_FALSE, &((ShadowRenderer::BIAS_MATRIX * shadowRenderer.sunCam.combined)[0][0]));

        earth.mesh->render();
        // DONE RENDERING WATER
*/
        spaceRenderer.renderBox(sunDirZoomedOut, cam, planetCamMovement.actualZoom);
/*
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

                          */

        glDisable(GL_BLEND);

        lineRenderer.projection = cam.combined;
         lineRenderer.line(glm::vec3(-300, 0, 0), glm::vec3(300, 0, 0), mu::X);
         lineRenderer.line(glm::vec3(0, -300, 0), glm::vec3(0, 300, 0), mu::Y);
         lineRenderer.line(glm::vec3(0, 0, -300), glm::vec3(0, 0, 300), mu::Z);

        // lineRenderer.line(shipPos, shipPos + vec3(0, 10, 0), mu::X);
        lvl->update(newDeltaTime);

//        for (int i = 0; i < 100; i += 2)
//            lineRenderer.line(sunDir * glm::vec3(i * 5), sunDir * glm::vec3((i + 1) * 5), glm::vec3(1, 1, 0));

        if (hoveredIsland)
        {
            // std::cout << hoveredIsland->percentageUnderwater() << "\n";
            lineRenderer.line(
                hoveredIsland->tileCenter(hoveredTile.x, hoveredTile.y),
                hoveredIsland->tileCenter(hoveredTile.x, hoveredTile.y) + hoveredIsland->vertexNormalsPlanet[hoveredIsland->xyToVertI(hoveredTile.x, hoveredTile.y)] * vec3(10),
                mu::X
            );
        }

        for (auto &l : isl->outlines2d)
        {
            auto prev = l.points[0];
            int limit = (time - 16) * 140;
//            int i = 0;
//            for (auto &p : l.points)
            for (int i = 1; i < limit && i < l.points.size(); i++)
            {
//                if (++i > limit) break;

                vec3 p0 = vec3(l.points[i - 1].x, 0., l.points[i - 1].y);
                vec3 p1 = vec3(l.points[i].x, 0., l.points[i].y);

                float newP = smoothstep<float>(23., 27., time);
                p0 *= 1. - newP;
                p1 *= 1. - newP;

                p0 += (isl->outlines3d[0][i - 1]) * newP;
                p1 += (isl->outlines3d[0][i]) * newP;

                lineRenderer.line(p0 + vec3(0, 0, 0), p1 + vec3(0, 0, 0), mu::X);
                lineRenderer.line(p0 + vec3(0, 0.1, 0), p1 + vec3(0, 0.1, 0), mu::X);
                lineRenderer.line(p0 + vec3(0, 0.2, 0), p1 + vec3(0, 0.2, 0), mu::X);
                lineRenderer.line(p0 + vec3(0, 0.3, 0), p1 + vec3(0, 0.3, 0), mu::X);
                lineRenderer.line(p0 + vec3(0, 0.4, 0), p1 + vec3(0, 0.4, 0), mu::X);
                lineRenderer.line(p0 + vec3(0, 0.5, 0), p1 + vec3(0, 0.5, 0), mu::X);
                lineRenderer.line(p0 + vec3(0, 0.6, 0), p1 + vec3(0, 0.6, 0), mu::X);
//                lineRenderer.line(vec3(prev.x, 0.7, prev.y), vec3(p.x, 0.7, p.y), mu::X);
//                lineRenderer.line(vec3(prev.x, 0.8, prev.y), vec3(p.x, 0.8, p.y), mu::X);
//                lineRenderer.line(vec3(prev.x, 0.9, prev.y), vec3(p.x, 0.9, p.y), mu::X);

//                prev = p;
            }
        }



//        for (Island *isl : lvl->earth.islands)
//        {
//            for (int x = 0; x < isl->width; x++)
//            {
//                for (int y = 0; y < isl->height; y++)
//                {
//                    auto b = isl->getBuilding(x, y);
//                    if (b)
//                    {
//                        vec3 n = isl->vertexNormalsPlanet[isl->xyToVertI(x, y)] * float (.1);
//                        lineRenderer.line(
//                                isl->vertexPositionsPlanet[isl->xyToVertI(x, y)] + n,
//                                isl->vertexPositionsPlanet[isl->xyToVertI(x + 1, y + 1)] + n,
//                                mu::X
//                        );
//                        lineRenderer.line(
//                                isl->vertexPositionsPlanet[isl->xyToVertI(x + 1, y)] + n,
//                                isl->vertexPositionsPlanet[isl->xyToVertI(x, y + 1)] + n,
//                                mu::X
//                        );
//                    }
//                }
//            }
//        }

        vec2 mouseLonLat(0);
        bool mouseOnEarth = earth.cursorToLonLat(&cam, mouseLonLat);

        Node nearestToMouse = lvl->seaGraph.nearest(mouseLonLat);

        std::vector<WayPoint> path;

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


//         for (auto &n : lvl->seaGraph.nodes)
//         {
//              for (auto &n2 : n->connections)
//              {
//                  lineRenderer.line(
//                      n->position * float(1.01), n2->position * float(1.01), vec3(1 - n->distToCoast / 5., .3, .8)
//                  );
//
//                  // std::cout << to_string(n->position) << " -> " << to_string(n2->position) << "\n";
//              }
////             if (n == nearestToMouse)
////                 lineRenderer.line(n->position, n->position * float(1.1), mu::Z);
//         }

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

    void renderShips(vec3 &sunDir, mat4 &view, bool reflection=false)
    {
        shipShader.use();
        glUniform1i(shipShader.location("reflection"), reflection);
        shipTexture->bind(0, shipShader, "shipTexture");
        if (reflection) glCullFace(GL_FRONT);

        for (auto &ship : lvl->ships) {
            auto &worldTrans = ship.transform;
            vec3 localSunDir = vec4(sunDir, 1) * worldTrans;
            glUniform3f(shipShader.location("sunDir"), localSunDir.x, localSunDir.y, localSunDir.z);
            glUniformMatrix4fv(shipShader.location("mvp"), 1, GL_FALSE, &(view * worldTrans)[0][0]);
            shipMesh->render();
        };
        glCullFace(GL_BACK);
    }

    void onResize()
    {
        cam.viewportWidth = gu::widthPixels;
        cam.viewportHeight = gu::heightPixels;
        cam.update();
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
