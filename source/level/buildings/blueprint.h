
#ifndef BLUEPRINTS_H
#define BLUEPRINTS_H

#include <string>
#include <functional>

typedef std::function<void(Building&)> BuildingGenerator;

struct Blueprint
{
    std::string name;
    const int width, height;

    const BuildingGenerator generator;

    const bool needsFlatGround = true;
};

/**
 * In BLUEPRINTS, all blueprints are available.
 * You can do BLUEPRINTS::PINE_TREE to access the blueprint of a pine tree.
 * You can also iterate over BLUEPRINTS::list to do something with all blueprints.
 *
 * Just don't read the code under this comment. It's a nasty macro hack.
 * All blueprints are defined in "./blueprint_list.inc"
 *
 * Each blueprint should have a generator function that is defined in building_generators.cpp
 * The generator function should look like this:
 *
 * void BLUEPRINTS::create_PINE_TREE(Building&);
 *
 * This function is responsible for initializing the building.
 * For example: create_PINE_TREE() will add the mesh of a Pine Tree to the 'building'. (yes, trees are buildings)
 */
namespace BLUEPRINTS {

#define BP(code_name, name, width, height, needsFlatGround) void create_##code_name(Building&);\
                                                            inline Blueprint code_name = Blueprint{name, width, height, create_##code_name, needsFlatGround};
#include "blueprint_list.inc"
#undef BP

inline std::vector<Blueprint*> list = {

#define BP(code_name, ...) &code_name,

#include "blueprint_list.inc"

#undef BP
};

};

#endif
