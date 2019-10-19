//#ifndef GAME_BUILDING_RENDERING_H
//#define GAME_BUILDING_RENDERING_H
//
//#include <optional>
//
//#include "../../../level.h"
//#include "graphics/texture.h"
//#include "../buildings.h"
//
//static const VertAttributes defaultBuildingVertAttrs = VertAttributes()
//                                                        .add_(VertAttributes::POSITION)
//                                                        .add_(VertAttributes::NORMAL)
//                                                        .add_(VertAttributes::TANGENT)
//                                                        .add_(VertAttributes::TEX_COORDS);
//
//class BuildingRenderingSystem;
//
//struct BuildingMeshVariant
//{
//    std::vector<SharedMesh> lodMeshes; // MUST BE UPLOADED TO SAME VertBuffer! lodMeshes[0] should have most detail
////    std::vector<SharedMesh> lodShadowMeshes; // must be uploaded to same VertBuffer as above.
//
//    SharedTexture texture;
//
//  private:
//    friend BuildingRenderingSystem;
//
//    int instancedVertDataId = -1;
//    VertData instanceTransforms = VertData(VertAttributes().add_(VertAttributes::TRANSFORM), std::vector<float>());
//    std::vector<entt::entity>
//        instances,
//        entitiesToAdd,
//        entitiesToRemove;
//    std::vector<unsigned int> indexesToRemove;
//};
//
//struct BuildingMesh_
//{
//    std::vector<BuildingMeshVariant> variants;
//    std::optional<ShaderProgram> customShaderProgram;
//};
//
//typedef std::shared_ptr<BuildingMesh_> BuildingMesh;
//
//struct RenderBuilding_cpnt
//{
//    BuildingMesh buildingMesh;
//    unsigned int variant;
//
//  private:
//    unsigned int instanceIndex; // remember the index so that the instance-data can be removed from BuildingMeshVariant.instanceTransforms
//    friend BuildingRenderingSystem;
//};
//
//class BuildingRenderingSystem : public LevelSystem
//{
//  public:
//    inline static BuildingRenderingSystem *active = NULL;
//
//    ShaderProgram defaultShader;
//
//        BuildingRenderingSystem(Level *lvl)
//        : defaultShader(
//                ShaderProgram::fromFiles("DefaultBuildingShader", "assets/shaders/building.vert", "assets/shaders/building.frag")
//          )
//    {
//        active = this;
////        lvl->registry.on_construct<RenderBuilding_cpnt>().connect<&BuildingRenderingSystem::onAdded>(this);
////        lvl->registry.on_destroy<RenderBuilding_cpnt>().connect<&BuildingRenderingSystem::onRemoved>(this);
//    }
//
//    void update(double deltaTime, Level *lvl) override
//    {}
//
//    void render(double deltaTime, Level *lvl)
//    {
//        defaultShader.use();
//        for (Island *isl : lvl->earth.islands)
//        {
//            if (!isl->isInView || !buildingMeshes.count(isl)) continue;
//
//            for (auto &buildingMesh : buildingMeshes[isl])
//            {
//                auto vertBuffer = buildingMesh->variants[0].lodMeshes[0]->vertBuffer;
//                for (auto &var : buildingMesh->variants)
//                {
//                    auto nrToAdd = var.entitiesToAdd.size(), nrToRemove = var.entitiesToRemove.size();
//                    if (nrToAdd || nrToRemove)
//                    {
//                        if (var.instancedVertDataId != -1)
//                            vertBuffer->deletePerInstanceData(var.instancedVertDataId);
//
//                        for (int i = 0; i < nrToRemove; i++)
//                        {
//                            var.instanceTransforms.setMat<mat4>(
//                                    var.instanceTransforms.getMat<mat4>(var.instances.size() - 1, 0),
//                                    var.indexesToRemove[i], 0
//                            );
//                            var.instanceTransforms.removeVertices(1);
//
//                            // remove instance from list:
//                            var.instances.erase(std::remove(var.instances.begin(), var.instances.end(), var.entitiesToRemove[i]), var.instances.end());
//                        }
//
//                        for (auto e : var.entitiesToAdd)
//                        {
//                            Placed_cpnt &placed = lvl->registry.get<Placed_cpnt>(e);
//                            var.instanceTransforms.addVertices(1);
//                            var.instanceTransforms.setMat<mat4>(placed.transform, var.instances.size(), 0);
//                            var.instances.push_back(e);
//                        }
//
//                        var.instancedVertDataId = vertBuffer->uploadPerInstanceData(var.instanceTransforms);
//                        var.entitiesToAdd.clear();
//                        var.entitiesToRemove.clear();
//                        var.indexesToRemove.clear();
//                    }
////                    vertBuffer->usePerInstanceData(var.instancedVertDataId);
//                    var.texture->bind(0, defaultShader, "buildingTexture");
//                    var.lodMeshes[0]->renderInstances(var.instances.size());
//                    std::cout << var.instances.size() << "\n";
//                }
//            }
//        }
//    }
//
//  private:
//    std::map<Island*, std::vector<BuildingMesh>> buildingMeshes;
//
//    void onAdded(entt::registry &reg, entt::entity entity, RenderBuilding_cpnt &rb)
//    {
//        if (!reg.has<Placed_cpnt>(entity)) return;
//
//        auto &placed = reg.get<Placed_cpnt>(entity);
//
//        std::cout << "Add building to instanced rendering on next render...\n";
//
//        if (!buildingMeshes.count(placed.on))
//            buildingMeshes[placed.on] = {};
//
//        auto &islMeshes = buildingMeshes[placed.on];
//        if (std::find(islMeshes.begin(), islMeshes.end(), rb.buildingMesh) == islMeshes.end())
//            islMeshes.push_back(rb.buildingMesh);
//
//        rb.buildingMesh->variants[rb.variant].entitiesToAdd.push_back(entity);
//    }
//
//    void onRemoved(entt::registry &reg, entt::entity entity)
//    {
//        std::cout << "bye\n";
//
//        RenderBuilding_cpnt &rb = reg.get<RenderBuilding_cpnt>(entity);
//        if (!rb.instanceIndex) return;
//
//        auto &var = rb.buildingMesh->variants[rb.variant];
//        var.entitiesToRemove.push_back(entity);
//        var.indexesToRemove.push_back(rb.instanceIndex);
//    }
//
//};
//
//#endif
