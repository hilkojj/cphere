
#include <iostream>
#include "gu/game_utils.h"
#include "level_screen.cpp"

#include "glm/glm.hpp"

int main()
{
    gu::Config config = {};
    // config.printOpenGLMessages = false;

    if (!gu::init(config))
        return -1;

    LevelScreen s;
    gu::setScreen(&s);

    gu::run();
    
    return 0;
}
