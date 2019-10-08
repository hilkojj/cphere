#ifndef PLANET_CAMERA_MOVEMENT_H
#define PLANET_CAMERA_MOVEMENT_H

#include <queue>

#include "graphics/3d/perspective_camera.h"
#include "planet.h"

class PlanetCameraMovement
{
  public:

    PlanetCameraMovement(PerspectiveCamera *cam, Planet *plt);

    void update(double deltaTime);

    float zoomVelocity = 0;
    float lon = 160, lat = 75, zoom = 0, actualZoom = 0;
    float horizonDistance = -1;

  private:
    void dragUpdate();
    vec2 dragVelocity() const;

    void updateHorizonDistance();

    PerspectiveCamera *cam;
    Planet *plt;

    float dragLon = 0, dragLat = 0, dragUpdateAccumulator = 0, afterDragTimer;
    bool accurateDraggingStarted = false;

    std::queue<vec2> dragHistory;
};


#endif
