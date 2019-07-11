#include "planet_camera_movement.h"
#include "input/mouse_input.h"
#include "gu/game_utils.h"

PlanetCameraMovement::PlanetCameraMovement(PerspectiveCamera *cam, Planet *plt) : cam(cam), plt(plt)
{
}

void PlanetCameraMovement::update(double deltaTime)
{
    bool startDrag = MouseInput::justPressed(GLFW_MOUSE_BUTTON_MIDDLE), dragging = MouseInput::pressed(GLFW_MOUSE_BUTTON_MIDDLE);
    if (dragging || startDrag)
    {
        bool lessAccurateMethod = false;
        vec2 cursorLonLat;
        if (plt->cursorToLonLat(cam, cursorLonLat))
        {
            if (!accurateDraggingStarted)
            {
                dragLon = cursorLonLat.x;
                dragLat = cursorLonLat.y;
                accurateDraggingStarted = true;
            }
            float actualLon = mod(270 - cursorLonLat.x, 360.f);
            if (abs(actualLon - lon) < 90 && cursorLonLat.y > 10 && cursorLonLat.y < 170)
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
            lon -= MouseInput::deltaMouseX * .2;
            lat -= MouseInput::deltaMouseY * .2;
        }
    }
    else
    {
        accurateDraggingStarted = false;
        bool
            up = MouseInput::mouseY < 35,
            down = MouseInput::mouseY > gu::height - 35,
            left = MouseInput::mouseX < 35,
            right = MouseInput::mouseX > gu::width - 35;

        if (up) lat -= deltaTime * 50;
        else if (down) lat += deltaTime * 50;

        if (left) lon -= deltaTime * 50;
        else if (right) lon += deltaTime * 50;
    }
    lon = mod(lon, 360.f);
    lat = max(0.f, min(180.f, lat));

    zoom = min(1. , max(0., zoom + MouseInput::yScroll * .15));

    cam->position = mu::Y * vec3(20. + 200 * (1. - zoom));
    cam->position.z += zoom * 20.;

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
}

