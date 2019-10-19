
#include "../../level.h"
#include "rendering/building_rendering.h"
#include "utils/json_model_loader.h"

namespace BLUEPRINTS
{

    void create_PINE_TREE(entt::entity e, entt::registry &reg)
    {
//        static BuildingMesh buildingMesh = NULL;
//
//        std::cout << "create Pine Tree\n";
//
//        if (!buildingMesh)
//        {
//            buildingMesh = BuildingMesh(new BuildingMesh_);
//            auto &variant = buildingMesh->variants.emplace_back();
//            {
//                SharedModel model = JsonModelLoader::fromUbjsonFile("assets/models/pine.ubj", &defaultBuildingVertAttrs)[0];
//                auto &pineMesh = model->parts[0].mesh;
//                VertBuffer::uploadSingleMesh(pineMesh);
//                variant.lodMeshes.push_back(pineMesh);
//                variant.texture = Texture::fromDDSFile("assets/textures/pine_tree.dds");
//            }
//        }
//        RenderBuilding_cpnt rb;
//        rb.buildingMesh = buildingMesh;
//        rb.variant = mu::randomInt(buildingMesh->variants.size() - 1);
//        reg.assign<RenderBuilding_cpnt>(e, rb);
    }

    void create_OAK_TREE(entt::entity e, entt::registry &reg)
    {
        std::cout << "create Oak Tree\n";
    }

}
