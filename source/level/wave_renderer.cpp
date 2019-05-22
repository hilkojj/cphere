#include "wave_renderer.h"
#include "graphics/3d/vert_buffer.h"
#include "glm/glm.hpp"
#include "utils/math_utils.h"
using namespace glm;

static VertAttributes attrs = VertAttributes() // keep this order because offsets are hardcoded in createWave().
                                    .add_({"POS_0", 3, GL_FALSE})
                                    .add_({"POS_1", 3, GL_FALSE})
                                    .add_({"WAVE_FRONT", 1, GL_FALSE})
                                    .add_({"OPACITY", 1, GL_FALSE});

WaveRenderer::WaveRenderer(Planet &earth)
    : earth(earth),
        shader(ShaderProgram::fromFiles("WavesShader", "assets/shaders/waves.vert", "assets/shaders/waves.frag"))
{
    for (auto isl : earth.islands) createWavesForIsland(isl);

    VertBuffer *buffer = VertBuffer::with(attrs);

    for (auto &islWaves : islandWaves)
        for (auto &wave : islWaves.waves) buffer->add(wave.mesh);
    buffer->upload(true);
}

void WaveRenderer::createWavesForIsland(Island *isl)
{
    for (auto &outline : isl->outlinesLongLat)
    {
        islandWaves.push_back({std::vector<Wave>(), isl});

        Polygon smoothed = Polygon(outline.points.size() / 3);
        for (int i = 0; i < smoothed.points.size(); i++)
            smoothed.points[i] = outline.points[i * 3];

        for (int x = 0; x < 10; x++)
            for (int i = 1; i < smoothed.points.size() - 1; i++)
                smoothed.points[i] = (smoothed.points[i - 1] + smoothed.points[i + 1]) / vec2(2);
        createWavesForOutline(islandWaves.back(), smoothed);
    }
}

void WaveRenderer::createWavesForOutline(IslandWaves &islWaves, Polygon &outline)
{
    int nrOfPoints = outline.points.size(); 
    Polygon offsetted = Polygon(nrOfPoints);
    offsetted.points[0] = offsetted.points[nrOfPoints - 1] = outline.points[0];

    // offset outline:
    for (int i = 1; i < nrOfPoints - 1; i++)
    {
        vec2 normal = vec2(0);

        for (int j = 1; j < 40; j++)
        {
            int min = i - j;
            min = (min % nrOfPoints + nrOfPoints) % nrOfPoints;

            normal += outline.points[min] - outline.points[(i + j) % nrOfPoints];
        }
        normal = normalize(vec2(normal.y, -normal.x));

        offsetted.points[i] = outline.points[i] + normal * vec2(4);

        if (outline.contains(offsetted.points[i].x, offsetted.points[i].y))
            offsetted.points[i] = outline.points[i] + normal * vec2(-5);

    }

    int nrOfWaves = nrOfPoints / 5;
    for (int i = 0; i < nrOfWaves; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            int waveLength = min(mu::randomInt(5, 45), nrOfPoints);
            int waveStart = mu::randomInt(0, nrOfPoints - waveLength);
            if (createWave(islWaves, outline, offsetted, waveStart, waveLength))
                break;
        }
    }
}

bool WaveRenderer::createWave(IslandWaves &islWaves, Polygon &outline, Polygon &offsetted, int waveStart, int waveLength)
{
    SharedMesh mesh = SharedMesh(new Mesh("island_wave", waveLength * 2, (waveLength - 1) * 6, attrs));

    for (int i = 0; i < waveLength; i++)
    {
        vec2 p0 = offsetted.points[i + waveStart],
            p1 = outline.points[i + waveStart];

        p1 -= (p0 - p1) * vec2(.3);

        for (int j = i + 1; j < waveLength; j++)
            if (mu::lineSegmentsIntersect(p0, p1, offsetted.points[j + waveStart], outline.points[j + waveStart]))
                return false;

        vec3 p3d0 = earth.lonLatTo3d(p0.x, p0.y, 0), p3d1 = earth.lonLatTo3d(p1.x, p1.y, 0);

        mesh->setVec3(p3d0, i * 2, 0);
        mesh->setVec3(p3d1, i * 2, 3);
        mesh->setVec3(p3d0, i * 2 + 1, 0);
        mesh->setVec3(p3d1, i * 2 + 1, 3);
        mesh->setFloat(0, i * 2, 6);
        mesh->setFloat(1, i * 2 + 1, 6);

        float opacity = min( // fading edges
            min(i / 3.0, 1.0),
            min((waveLength - i - 2) / 3.0, 1.0)
        );
        mesh->setFloat(opacity, i * 2, 7);
        mesh->setFloat(opacity, i * 2 + 1, 7);

        if (i >= waveLength - 2) continue;
        int index = i * 6;
        mesh->indices[index] = i * 2;
        mesh->indices[index + 1] = i * 2 + 1;
        mesh->indices[index + 2] = i * 2 + 2;

        mesh->indices[index + 3] = i * 2 + 1;
        mesh->indices[index + 4] = i * 2 + 3;
        mesh->indices[index + 5] = i * 2 + 2;
    }
    islWaves.waves.push_back({mesh, 0});
    return true;
}

void WaveRenderer::render(double deltaTime, const glm::mat4 &view)
{
    shader.use();
    // glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ZERO, GL_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glUniformMatrix4fv(shader.location("view"), 1, GL_FALSE, &view[0][0]);
    for (auto &islWaves : islandWaves)
    {
        if (!islWaves.isl->isInView) continue;

        for (auto &wave : islWaves.waves)
        {
            if (wave.timer == 0 && mu::random() < .03 * deltaTime) wave.timer = .0001;
            if (wave.timer == 0) continue;

            glUniform1f(shader.location("timer"), wave.timer);
            wave.mesh->render();

            wave.timer += deltaTime * .15;
            if (wave.timer > 1) wave.timer = 0;
        }
    }
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
}

