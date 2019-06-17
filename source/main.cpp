
#include <iostream>
#include "gu/game_utils.h"
#include "level_screen.cpp"
#include "utils/math_utils.h"

#include "serialization.h"

#include "graphics/texture_array.h"

int main()
{
    gu::Config config = {};
    config.printOpenGLMessages = true;
    config.samples = 0;
    #ifdef EMSCRITPEN
    config.vsync = true;
    #endif

    if (!gu::init(config))
        return -1;

    // TextureArray::fromDDSFiles({"assets/textures/sun/flare0.dds", "assets/textures/sun/flare0.dds"});

    LevelScreen s;
    gu::setScreen(&s);

    gu::run();



    return 0;
}
