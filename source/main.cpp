
#include <iostream>
#include "gu/game_utils.h"
#include "level_screen.cpp"
#include "utils/math_utils.h"

int main()
{
    gu::Config config = {};
    config.printOpenGLMessages = false;
    config.samples = 0;

    if (!gu::init(config))
        return -1;

    LevelScreen s;
    gu::setScreen(&s);

    gu::run();
    return 0;
}
