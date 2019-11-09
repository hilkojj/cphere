
#include <iostream>
#include "gu/game_utils.h"
#include "level_screen.cpp"

// #include "../external/ctpl_stl.h"

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

    #ifdef EMSCRIPTEN
    const char *path = "assets/pre-generated-level.save";
    #else
    char *path = NULL;
    for (int i = 0; i < argc; i++)
        if (std::string(argv[i]) == "-s")
            path = argv[i + 1];
    #endif

    LevelScreen s(path);
    gu::setScreen(&s);

    gu::run();
    return 0;
}
