
#ifndef SHIP_WAKE_H
#define SHIP_WAKE_H

#include "graphics/3d/mesh.h"
#include "graphics/3d/debug_line_renderer.h"
#include "utils/math_utils.h"

class ShipWake
{
  public:
    ShipWake();

    SharedMesh mesh;

    void render(DebugLineRenderer &lineRenderer, vec3 &pos, double deltaTime);

  private:
    vec3 prevPos, prevDir = mu::X;

    float timeLeft = 1.;
};

#endif
