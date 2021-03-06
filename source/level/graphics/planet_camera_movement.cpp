#include "planet_camera_movement.h"
#include "input/mouse_input.h"
#include "gu/game_utils.h"

PlanetCameraMovement::PlanetCameraMovement(Camera *cam, Planet *plt)
    : cam(cam), plt(plt), slightlySmallerPlt("PltCamMovementPlt", Sphere(plt->sphere.radius - 10.))
{
}

const double DRAG_UPDATE_STEP = 1. / 60.;
const int DRAG_BUTTON = GLFW_MOUSE_BUTTON_LEFT;

void PlanetCameraMovement::update(double deltaTime, Planet *plttt)
{
    auto prevPos = cam->position;
    plt = plttt;
    bool startDrag = MouseInput::justPressed(DRAG_BUTTON),
         dragging = MouseInput::pressed(DRAG_BUTTON),
         stoppedDragging = MouseInput::justReleased(DRAG_BUTTON);
    if (dragging || startDrag)
    {
        while (startDrag && !dragHistory.empty()) dragHistory.pop();
        
        // dragUpdateAccumulator += deltaTime;

//        while (dragUpdateAccumulator > DRAG_UPDATE_STEP)
//        {
        dragUpdate();
//            dragUpdateAccumulator -= DRAG_UPDATE_STEP;
//        }
    }
    else
    {
        accurateDraggingStarted = false;
        bool
            up = MouseInput::mouseY < 35,
            down = MouseInput::mouseY > gu::height - 35,
            left = MouseInput::mouseX < 35,
            right = MouseInput::mouseX > gu::width - 35;

        float moveSpeed = 50. - actualZoom * 20.;

        if (up) lat -= deltaTime * moveSpeed;
        else if (down) lat += deltaTime * moveSpeed;

        if (left) lon -= deltaTime * moveSpeed;
        else if (right) lon += deltaTime * moveSpeed;

        if (stoppedDragging) afterDragTimer = .5;

        if (afterDragTimer > 0)
        {
            vec2 d = dragVelocity() * vec2(afterDragTimer * deltaTime * 20.);
            lon += d.x;
            lat += d.y;
            afterDragTimer -= deltaTime;
        }
    }
    lon = mod(lon, 360.f);
    lat = max(0.f, min(180.f, lat));

    zoom = min(.92 , max(0., zoom + MouseInput::yScroll * .08));
    float prevActualZoom = actualZoom;
    actualZoom = actualZoom * (1. - deltaTime * 10.) + zoom * deltaTime * 10.;

    zoomVelocity = abs(prevActualZoom - actualZoom) / deltaTime;

    cam->position = mu::Y * vec3(5. + 235 * (1. - actualZoom));
    cam->position.z += actualZoom * 24.5;

    cam->lookAt(mu::ZERO_3, -mu::Z);

    vec3 translateCam = vec3(0, plt->sphere.radius, 0);

    mat4 transform(1);
    transform = rotate(transform, lon * mu::DEGREES_TO_RAD, mu::Y);
    transform = rotate(transform, lat * mu::DEGREES_TO_RAD, mu::X);
    transform = translate(transform, translateCam);

    cam->position = transform * vec4(cam->position, 1);
    cam->direction = transform * vec4(cam->direction, 0);
    cam->right = transform * vec4(cam->right, 0);
    cam->up = transform * vec4(cam->up, 0);

    cam->update();
    if (prevPos != cam->position) islandFrustumCulling();
}

void PlanetCameraMovement::dragUpdate()
{
    bool lessAccurateMethod = false;
    vec2 cursorLonLat;
    if (slightlySmallerPlt.cursorToLonLat(cam, cursorLonLat))
    {
        plt->cursorToLonLat(cam, cursorLonLat);
        if (!accurateDraggingStarted)
        {
            dragLon = cursorLonLat.x;
            dragLat = cursorLonLat.y;
            accurateDraggingStarted = true;
        }
        float actualLon = mod(270 - cursorLonLat.x, 360.f);
        float lonDiff = abs(actualLon - lon);
        lonDiff = min(lonDiff, 360 - lonDiff);
        if (lonDiff < 90 && cursorLonLat.y > 15 && cursorLonLat.y < 165)
        {
            float deltaLon = cursorLonLat.x - dragLon,
                deltaLat = cursorLonLat.y - dragLat;

            lon += deltaLon;
            lat -= deltaLat;

            dragLon = cursorLonLat.x - deltaLon;
            dragLat = cursorLonLat.y - deltaLat;

        }
        else lessAccurateMethod = true;
    }
    else lessAccurateMethod = true;

    if (lessAccurateMethod)
    {
        accurateDraggingStarted = false;
        lon -= MouseInput::deltaMouseX * .13;
        lat -= MouseInput::deltaMouseY * .13;
    }
    dragHistory.push(vec2(lon, lat));
    if (dragHistory.size() > 10)
        dragHistory.pop();
}

vec2 PlanetCameraMovement::dragVelocity() const
{
    if (dragHistory.size() == 0) return vec2(0);
    return Planet::deltaLonLat(dragHistory.front(), vec2(lon, lat)) / vec2(dragHistory.size());
}


void PlanetCameraMovement::updateHorizonDistance()
{
    horizonDistance = sqrt(pow(length(cam->position), 2) - pow(plt->sphere.radius, 2));
}

void PlanetCameraMovement::islandFrustumCulling()
{
    updateHorizonDistance();
    int nrInView = 0;
    for (Island *isl : plt->islands)
    {
        isl->isInView = false;
        for (int x = 0; x < isl->width; x += 10)
        {
            for (int y = 0; y < isl->height && !isl->isInView; y += 10)
            {
                if (isl->tileAtSeaFloor(x, y)) continue;

                vec3 p = isl->vertexPositionsPlanet[isl->xyToVertI(x, y)];

                if (length(p - cam->position) > horizonDistance + 15) continue;

                bool inView = false;
                cam->project(p, inView);
                if (inView) isl->isInView = true;
            }
        }
        if (isl->isInView) nrInView++;
    }
}

