
#include "ship_wake.h"
#include "graphics/3d/vert_buffer.h"

#include <memory.h>

const int NR_OF_LINES = 40;

ShipWake::ShipWake()
{
    mesh = SharedMesh(new Mesh("ship_wake", NR_OF_LINES * 2, (NR_OF_LINES - 1) * 6, VertAttributes().add_(VertAttributes::POSITION).add_(VertAttributes::TEX_COORDS)));

    for (int i = 0; i < NR_OF_LINES - 1; i++)
    {
        mesh->indices[i * 6 + 0] = 2 + i * 2;
        mesh->indices[i * 6 + 1] = 1 + i * 2;
        mesh->indices[i * 6 + 2] = 0 + i * 2;

        mesh->indices[i * 6 + 3] = 3 + i * 2;
        mesh->indices[i * 6 + 4] = 1 + i * 2;
        mesh->indices[i * 6 + 5] = 2 + i * 2;
    }
    for (int i = 0; i < NR_OF_LINES; i++)
    {
        mesh->setVec3(vec3(0), i * 2, 0);
        mesh->setVec3(vec3(0), i * 2 + 1, 0);

        float y = i / (NR_OF_LINES - 1.);

        mesh->setVec2(vec2(0, y), i * 2, 3);
        mesh->setVec2(vec2(1, y), i * 2 + 1, 3);
    }
    VertBuffer::uploadSingleMesh(mesh);
}

void ShipWake::render(DebugLineRenderer &lineRenderer, vec3 &pos, double deltaTime)
{
    // temp:
    delete mesh->vertBuffer;
    mesh->vertBuffer = NULL;

    vec3 dir = pos - prevPos;
    if (length(dir) < .02)
    {
        dir = prevDir;
    } else {
        dir = normalize(dir + prevDir);
        prevDir = dir;
    }

    vec3 dirTangent = glm::rotate(dir, mu::DEGREES_TO_RAD * 90, pos); // todo

    timeLeft -= deltaTime;
    if (timeLeft <= 0)
    {
        for (int i = NR_OF_LINES - 1; i >= 1; i--)
        {
            mesh->setVec3(mesh->getVec3(i * 2 - 2, 0), i * 2, 0);
            mesh->setVec3(mesh->getVec3(i * 2 - 1, 0), i * 2 + 1, 0);
        }
        // memcpy(&mesh->vertices[2], &mesh->vertices[0], sizeof(float) * mesh->attributes.getVertSize() * (NR_OF_LINES - 1) * 2);

        timeLeft = .1;
    }
    // set first line position to ship position
    mesh->setVec3(pos + dirTangent * vec3(.1), 0, 0);
    mesh->setVec3(pos - dirTangent * vec3(.1), 1, 0);

    // lineRenderer.line(mesh->getVec3(0, 0), mesh->getVec3(0, 0) + vec3(0, 10, 0), mu::Z);
    // lineRenderer.line(mesh->getVec3(1, 0), mesh->getVec3(1, 0) + vec3(0, 10, 0), mu::Z);

    for (int i = 1; i < NR_OF_LINES; i++)
    {
        // note: loop starts at second line! (i = 1)

        vec3 p0 = mesh->getVec3(i * 2, 0), p1 = mesh->getVec3(i * 2 + 1, 0);


        // lineRenderer.line(p0, p0 + vec3(0, 4, 0), mu::Y);
        // lineRenderer.line(p1, p1 + vec3(0, 4, 0), mu::Y);

        vec3 lineTangent = normalize(p0 - p1);

        mesh->setVec3(p0 + lineTangent * vec3(2. * deltaTime), i * 2, 0);
        mesh->setVec3(p1 - lineTangent * vec3(2. * deltaTime), i * 2 + 1, 0);
    }

    // temp:
    VertBuffer::uploadSingleMesh(mesh);

    prevPos = pos;

    glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ZERO, GL_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    mesh->render();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
}
