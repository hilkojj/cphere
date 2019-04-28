
#include <iostream>
#include "gu/game_utils.h"

int main()
{
    gu::Config config = {};

    if (!gu::init(config))
        return -1;

    gu::run();
    
    return 0;
}
