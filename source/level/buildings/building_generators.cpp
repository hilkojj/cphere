
#include <graphics/3d/vert_buffer.h>
#include <utils/math_utils.h>
#include "building.h"
#include "utils/json_model_loader.h"

namespace BLUEPRINTS
{

    void create_PINE_TREE(Building b)
    {
        static BuildingMesh *buildingMesh = NULL;

        std::cout << "create Pine Tree\n";

        if (!buildingMesh)
        {
            buildingMesh = new BuildingMesh{};
            auto &variant = buildingMesh->variants.emplace_back();
            {
                SharedModel model = JsonModelLoader::fromUbjsonFile("assets/models/pine.ubj", &defaultBuildingVertAttrs)[0];
                auto &pineMesh = model->parts[0].mesh;
                VertBuffer::uploadSingleMesh(pineMesh);
                variant.lodMeshes.push_back(pineMesh);
                variant.texture = Texture::fromDDSFile("assets/textures/pine_tree.dds");
            }
        }
        RenderBuilding rb;
        rb.buildingMesh = buildingMesh;
        rb.variant = mu::randomInt(buildingMesh->variants.size() - 1);
        b->renderBuilding = rb;
    }

    void create_OAK_TREE(Building b)
    {
        std::cout << "create Oak Tree\n";
    }

}
