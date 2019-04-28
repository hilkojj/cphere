
#include <iostream>
#include "graphics/3d/perspective_camera.h"
#include "utils/math_utils.h"
#include "utils/json_model_loader.h"

int main()
{
    PerspectiveCamera cam(1, 100, 1600, 900, 75);

    cam.rotate(10, mu::X);
	std::cout << cam.fOV << "hello\n";

    SharedModel m = JsonModelLoader::fromJsonFile("bla", NULL)[0];

	return 0;
}

