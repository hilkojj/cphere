
#include <iostream>
#include "gu/game_utils.h"
#include "level_screen.cpp"

#include "../external/ctpl_stl.h"

int main(int argc, char *argv[])
{
    gu::Config config = {};
    config.printOpenGLMessages = false;
    config.samples = 0;
    #ifdef EMSCRIPTEN
    config.vsync = true;
    #endif
    // config.vsync = true;

    if (!gu::init(config))
        return -1;

    char *path = NULL;
    for (int i = 0; i < argc; i++)
        if (std::string(argv[i]) == "-s")
            path = argv[i + 1];

    LocalServer svr({Planet("earth", Sphere(150.))}, path);

    LevelScreen s(&svr);
    gu::setScreen(&s);

    gu::run();
    return 0;
}


/*


| server update       | client update |
                      | client update |
                      | client update |
| render clouds etc.  | render            |
                

*/
