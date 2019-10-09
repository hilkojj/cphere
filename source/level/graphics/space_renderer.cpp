
#include "space_renderer.h"
#include "graphics/3d/vert_buffer.h"
#include "utils/json_model_loader.h"
#include "utils/math_utils.h"
#include "gu/game_utils.h"
#include "../planet.h"

SpaceRenderer::SpaceRenderer()
    : cubeMapShader(ShaderProgram::fromFiles(
        "SpaceCubeMapShader",
        "assets/shaders/space_box.vert",
        "assets/shaders/space_box.frag"
    )),
    cubeMap(CubeMap::fromDDSFiles({
        "assets/textures/space_cubemap/right.dds",
        "assets/textures/space_cubemap/left.dds",
        "assets/textures/space_cubemap/top.dds",
        "assets/textures/space_cubemap/bottom.dds",
        "assets/textures/space_cubemap/front.dds",
        "assets/textures/space_cubemap/back.dds",
    })),
    sunTexture(Texture::fromDDSFile("assets/textures/sun/sun.dds")),

    flareTextures(TextureArray::fromDDSFiles({
        "assets/textures/sun/flare1.dds",
        "assets/textures/sun/flare0.dds",
        "assets/textures/sun/flare3.dds",
        "assets/textures/sun/flare4.dds",
        "assets/textures/sun/flare5.dds"
    })),
    sunShader(ShaderProgram::fromFiles(
        "SunShader",
        "assets/shaders/sun_and_flare.vert",
        "assets/shaders/sun.frag"
    )),
    flareShader(ShaderProgram::fromFiles(
        "FlareShader",
        "assets/shaders/sun_and_flare.vert",
        "assets/shaders/flare.frag"
    ))
{
    cube = JsonModelLoader::fromUbjsonFile("assets/models/cube.ubj", &VertAttributes().add_(VertAttributes::POSITION))[0]->parts[0].mesh;
    VertBuffer::uploadSingleMesh(cube);
}

void SpaceRenderer::renderBox(const vec3 &sunDir, const Camera &cam, float zoom)
{
    glDisable(GL_BLEND);
    cubeMapShader.use();
    cubeMap->bind(0);
    glUniform1i(cubeMapShader.location("cubemap"), 0);
    glUniform1f(cubeMapShader.location("zoomedIn"), zoom);

    mat4 view = rotate(cam.combined, (float) -atan2(sunDir.z, sunDir.x) + mu::PI * .5f, mu::Y);

    glUniformMatrix4fv(cubeMapShader.location("view"), 1, GL_FALSE, &view[0][0]);
    glCullFace(GL_FRONT);
    glDepthMask(false);
    glDepthFunc(GL_LEQUAL);
    cube->render();
    glCullFace(GL_BACK);
    glDepthMask(true);
    glDepthFunc(GL_LESS);
}

const LensFlare SpaceRenderer::flares[] = {
    {4, vec4(1), 1, 1, false},

    {1, vec4(1, 1, 1, .35), 1, -.8, true},

    {3, vec4(.7, .8, .9, .05), .25, -1.6, false},
    {3, vec4(.9, .8, .4, .1), .38, -1.5, false},
    {3, vec4(.7, .9, .8, .15), .45, -1.4, false},

    {3, vec4(.9, .9, .7, .1), .18, -.75, false},

    {0, vec4(.9, .9, .7, .7), .05, -.74, false},
    {0, vec4(1), .015, -.75, false},

    {0, vec4(.2, .3, 2, .7), .05, -.8, false},
    {0, vec4(1), .015, -.81, false},

    {3, vec4(.9, .9, .7, .07), .5, -.5, false},
    {3, vec4(.9, .9, .7, .03), .8, -.54, false},

    {3, vec4(.1, 1, .2, .2), .1, -.5, false},
    {3, vec4(.1, 1, .2, .2), .09, -.47, false},
    {0, vec4(.7, 1, .7, 1), .008, -.48, false},

    {0, vec4(.1, .7, 1, .4), .06, .1, false},
    {0, vec4(.7, 1, .7, 1), .008, .11, false},

    {3, vec4(.8, .8, .3, .2), .06, .23, false},
    {0, vec4(1, 1, .7, 1), .008, .225, false},

    {0, vec4(.6, 1, .8, 1), .1, .23, false},

    {2, vec4(1, .3, .1, .2), .5, 1.03, false},
    {3, vec4(.6, .4, 1, .15), .4, 1.55, false},

    {0, vec4(.2, 1, .5, .3), .03, 1.8, false},
    {0, vec4(.2, 1, .5, 1), .008, 1.81, false},
};

void SpaceRenderer::renderSun(const vec3 &sunDir, const Camera &cam, SharedTexture depth, float time, const Planet &plt)
{
    sunShader.use();
    sunTexture->bind(0, sunShader, "sun");
    depth->bind(1, sunShader, "depthTex");

    mat4 modelT = translate(translate(mat4(1), cam.position), sunDir * vec3(5));
    modelT = rotate(modelT, (float) -atan2(sunDir.z, sunDir.x) + mu::PI * -.5f, mu::Y);

    glUniformMatrix4fv(sunShader.location("mvp"), 1, GL_FALSE, &(cam.combined * modelT)[0][0]);
    glUniform2f(sunShader.location("scrSize"), gu::widthPixels, gu::heightPixels);
    glUniform1f(sunShader.location("time"), time);
    glDisable(GL_DEPTH_TEST);
    Mesh::getQuad()->render();

    // lensflare:
    float lensFlareA = 1;

    int steps = 50;

    for (int i = 0; i < steps; i++)
    {
        vec3 sd = rotate(sunDir, (float) (.05 * (((float) i - steps * .5) / steps)), mu::Y);
        vec3 sunPos = cam.position + sd;

        bool inViewport = false;
        cam.project(sunPos, inViewport);
        
        if (!inViewport || plt.sphere.rayIntersection(cam.position, sd, NULL, NULL))
            lensFlareA -= 1.0 / steps;

    }
    lensFlareA = pow(lensFlareA, 2.);
    lensFlareA = lensFlareAlpha = (lensFlareA + lensFlareAlpha) * .5;

    vec2 screenSunPos = cam.project(cam.position + sunDir);

    lensFlareA *= 1.4 - length(screenSunPos);
    lensFlareA *= .45;
    flareShader.use();
    glBlendFunc(GL_ONE, GL_ONE);
    flareTextures->bind(0);
    glUniform1i(flareShader.location("textures"), 0);

    mat4 correctScale = scale(mat4(1), vec3(((float) gu::height / gu::width), 1, 1));

    for (auto &flare : flares)
    {
        mat4 mvp = translate(scale(mat4(1), vec3(flare.scale)), vec3(screenSunPos * flare.dist / flare.scale, 1)) * correctScale;

        if (flare.rotate)
        {
            mvp = rotate(
                mvp,
                (float) -atan2(screenSunPos.x, screenSunPos.y) + mu::PI * -1.25f,
                mu::Z
            );
        }
	    
        glUniformMatrix4fv(flareShader.location("mvp"), 1, GL_FALSE, &(mvp)[0][0]);
        glUniform1i(flareShader.location("layer"), flare.texture);
        glUniform4f(flareShader.location("flareColor"), flare.color.r, flare.color.g, flare.color.b, flare.color.a * lensFlareA);

        Mesh::getQuad()->render();
    }
    glEnable(GL_DEPTH_TEST);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

