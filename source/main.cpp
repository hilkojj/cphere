
#include <iostream>
#include "gu/game_utils.h"
#include "level_screen.cpp"

int main(int argc, char *argv[])
{
    gu::Config config = {};
    config.printOpenGLMessages = false;
    config.samples = 0;

    if (!gu::init(config))
        return -1;

    bool load = false;
    char *path = NULL;
    for (int i = 0; i < argc; i++)
    {
        if (std::string(argv[i]) == "-s")
        {
            load = true;
            path = argv[i + 1];
        }
    }

    LevelScreen s(load, path);
    gu::setScreen(&s);

    gu::run();
    return 0;
}
