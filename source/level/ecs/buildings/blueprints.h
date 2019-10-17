
#ifndef BLUEPRINTS_H
#define BLUEPRINTS_H

typedef std::function<void(entt::entity, entt::registry&)> BuildingGenerator;

struct Blueprint_
{
    std::string name;
    const int width, height;

    const BuildingGenerator generator;

    const bool needsFlatGround = true;
};

typedef std::shared_ptr<Blueprint_> Blueprint;


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
 * void BLUEPRINTS::create_PINE_TREE(entt::entity, entt::registry&);
 *
 * This function is responsible for adding components to the entity.
 * For example: create_PINE_TREE() adds a Tree component and a Mesh component to the entity.
 * Another example: create_HOUSE() adds a House component and a Mesh component to the entity.
 */
namespace BLUEPRINTS {

#define BP(code_name, name, width, height, needsFlatGround) void create_##code_name(entt::entity, entt::registry&);\
                                                            inline Blueprint code_name = Blueprint(new Blueprint_{name, width, height, create_##code_name, needsFlatGround});
#include "blueprint_list.inc"
#undef BP

inline std::vector<Blueprint> list = {

#define BP(code_name, ...) code_name,

#include "blueprint_list.inc"

#undef BP
};

};

#endif
