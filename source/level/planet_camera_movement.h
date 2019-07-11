#ifndef PLANET_CAMERA_MOVEMENT_H
#define PLANET_CAMERA_MOVEMENT_H

#include "graphics/3d/perspective_camera.h"
#include "planet.h"

class PlanetCameraMovement
{
  public:

    PlanetCameraMovement(PerspectiveCamera *cam, Planet *plt);

    void update(double deltaTime);

  private:
    PerspectiveCamera *cam;
    Planet *plt;

    float lon = 0, lat = 90, zoom = 0;

    float dragLon = 0, dragLat = 0;
    bool accurateDraggingStarted = false;

};


#endif
