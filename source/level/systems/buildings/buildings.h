//#ifndef GAME_BUILDINGS_H
//#define GAME_BUILDINGS_H
//
//#include <string>
//#include <memory>
//#include <map>
//#include <functional>
//#include <input/key_input.h>
//#include "utils/gu_error.h"
//#include "utils/math_utils.h"
//#include "graphics/3d/mesh.h"
//#include "input/mouse_input.h"
//#include "../../level.h"
//
//struct Placed_cpnt
//{
//    Blueprint bp;
//    Island *on;
//    int rotation; // how many times the building is rotated (90 degrees)
//    std::vector<ivec2> tiles; // the tiles occupied by the building
//    mat4 transform;
//
//    Placed_cpnt(Blueprint &bp, int x, int y, int rotation, Island *on, mat4 transform=mat4(0))
//
//        : bp(bp), rotation(rotation), on(on), transform(transform), tiles(bp->width * bp->height)
//    {
//        int i = 0;
//        bool rot = rotation % 2 == 0;
//        for (int x0 = 0; x0 < bp->width; x0++)
//            for (int y0 = 0; y0 < bp->height; y0++)
//                tiles[i++] = ivec2(x + (rot ? x0 : y0), y + (rot ? y0 : x0));
//
//        if (transform == mat4(0))
//        {
//            // calculate transform:
//            transform = mat4(1);
//        }
//    }
//};
//
//struct Placing_cpnt
//{
//    Island *on = NULL;
//    int x = 0, y = 0, rotation = 0;
//    bool blocked = false;
//};
//
//class BuildingSystem : public LevelSystem
//{
//  public:
//    BuildingSystem(Level *lvl)
//    {
////        lvl->registry.on_construct<Placed_cpnt>().connect<&BuildingSystem::onPlaced>(this);
////        lvl->registry.on_destroy<Placed_cpnt>().connect<&BuildingSystem::onDestroyed>(this);
//
////        for (auto bp : BLUEPRINTS::list)
////        {
////            auto bpEntity = lvl->registry.create();
////            bp->generator(bpEntity, lvl->registry);
////            lvl->registry.assign<Blueprint>(bpEntity, bp);
////        }
//    }
//
//    void update(double deltaTime, Level *lvl) override
//    {
//        int i = 0;
////        lvl->registry.view<Blueprint>().each([&](auto e, Blueprint &bp) {
////            if (KeyInput::justPressed(GLFW_KEY_1 + i++))
////            {
////                lvl->registry.reset<Placing_cpnt>();
////                lvl->registry.assign<Placing_cpnt>(e);
////                std::cout << "Start placing of " << bp->name << "\n";
////            }
////        });
//
//        if (KeyInput::justPressed(GLFW_KEY_DELETE))
//        {
//            auto *isl = lvl->earth.islUnderCursor(lvl->cam);
//            if (isl)
//            {
//                ivec2 tile;
//                if (isl->tileUnderCursor(tile, lvl->cam))
//                {
//                    auto e = isl->getBuilding(tile.x, tile.y);
//                    if (e)
//                        lvl->registry.destroy(e);
//                }
//            }
//        }
//
////        lvl->registry.view<Placing_cpnt, Blueprint>().each([&](auto entity, Placing_cpnt &placing, Blueprint &bp) {
////
////            placing.blocked = true;
////            auto *isl = lvl->earth.islUnderCursor(lvl->cam);
////            if (isl)
////            {
////                placing.on = isl;
////                ivec2 tile;
////                if (isl->tileUnderCursor(tile, lvl->cam))
////                {
////                    placing.x = tile.x;
////                    placing.y = tile.y;
////
////                    placing.blocked = !canPlace(bp, placing.x, placing.y, placing.rotation, placing.on, lvl->registry);
////                }
////            }
////            if (MouseInput::justPressed(GLFW_MOUSE_BUTTON_LEFT))
////            {
////                if (placing.blocked)
////                    std::cout << "BUILDING BLOCKED!" << "\n";
////                else {
////                    place(bp, lvl->registry, placing.x, placing.y, placing.rotation, placing.on);
////                    // stop placing building
////                    lvl->registry.reset<Placing_cpnt>();
////                }
////            }
////        });
//    }
//
//    static bool canPlace(Blueprint &bp, int x, int y, int rotation, Island *isl, entt::registry &reg)
//    {
//        Placed_cpnt p(bp, x, y, rotation, isl);
//        for (auto &t : p.tiles)
//        {
//            if (!isl->containsTile(t.x, t.y)) return false;
//            if (!isl->tileAboveSea(t.x, t.y)) return false;
//            if (isl->getBuilding(t.x, t.y)) return false;
//            if (bp->needsFlatGround && isl->tileSteepness(t.x, t.y) > 1) return false;
//        }
//        return true;
//    }
//
//    static void place(Blueprint &bp, entt::registry &reg, int x, int y, int rotation, Island *on)
//    {
//        std::cout << "PLACING " << bp->name << "\n";
//
//        auto newEntity = reg.create();
//        reg.assign<Placed_cpnt>(newEntity, bp, x, y, rotation, on);
//
//        bp->generator(newEntity, reg);
//
//        for (auto &bp : BLUEPRINTS::list)
//        {
//            std::cout << bp->name << " " << bp.use_count() << "\n";
//        }
//    }
//
//  private:
//    void onPlaced(entt::registry &reg, entt::entity entity, Placed_cpnt &placed)
//    {
//        std::cout << placed.bp->name << " placed at " << to_string(placed.tiles[0]) << " on Island " << placed.on << "\n";
//
//        for (auto &p : placed.tiles)
//        {
//            if (placed.on->buildings[p.x][p.y])
//                throw gu_err("ATTEMPT TO PLACE BUILDING ON ALREADY OCCUPIED TILE");
//
//            placed.on->buildings[p.x][p.y] = entity;
//        }
//    }
//
//    void onDestroyed(entt::registry &reg, entt::entity entity)
//    {
//        auto &placed = reg.get<Placed_cpnt>(entity);
//        std::cout << placed.bp->name << " DESTROYED at " << to_string(placed.tiles[0]) << " on Island " << placed.on << "\n";
//
//        for (auto &p : placed.tiles)
//        {
//            if (placed.on->buildings[p.x][p.y] != entity)
//                throw gu_err("ATTEMPT TO REMOVE BUILDING THAT WAS NOT PLACED ON SAID TILES");
//
//            placed.on->buildings[p.x][p.y] = 0;
//        }
//    }
//};
//
//#endif
