
#include <iostream>
#include "gu/game_utils.h"
#include "level_screen.cpp"
#include "utils/math_utils.h"

#include "serialization.h"

#include <bitset>

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

    // std::cout << slz::float_m4::size << "\n";

    // for (float a = -1; a < 1; a += .01)
    // {

    //     auto b = slz::Float<uint8, 1, -1>::from(a);

    //     std::cout << a << "\n";
    //     std::cout << b << "\n";
    //     std::cout << slz::Float<uint8, 1, -1>::get(b) << "\n\n";
    // }

    // std::vector<unsigned char> bla;
    // // slz::Float<uint16>::serializeVec(vec3(.123, .234, .345), bla);

    // // vec3 poepie;

    // // slz::Float<uint16>::deserializeVec(&bla[0], poepie);

    // // std::cout << to_string(poepie) << "\n";


    // std::vector<vec3> vecs = {vec3(.12, .23, .34), vec3(.45, .56, .67), vec3(.78, .89, .90)};

    // slz::Float<uint8>::serializeVecs(vecs, bla);

    // File::writeBinary("test.bin", bla);

    // auto data = File::readBinary("test.bin");

    // std::vector<vec3> vecsb;

    // slz::Float<uint8>::deserializeVecs(&data[0], vecs.size(), vecsb);

    // for (auto &v : vecsb)
    //     std::cout << to_string(v) << "\n";

    return 0;
}
