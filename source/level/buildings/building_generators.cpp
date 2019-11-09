
#include <graphics/3d/vert_buffer.h>
#include <utils/math_utils.h>
#include "building.h"
#include "utils/json_model_loader.h"

namespace BLUEPRINTS
{

    void create_PINE_TREE(Building &b)
    {
        static BuildingMesh *buildingMesh = NULL;

        std::cout << "create Pine Tree\n";

        if (!buildingMesh)
        {
            buildingMesh = new BuildingMesh{ TREE };
            auto &variant = buildingMesh->variants.emplace_back();
            {
                SharedModel model = JsonModelLoader::fromUbjsonFile("assets/models/pine.ubj", &DEFAULT_BUILDING_VERT_ATTRS)[0];
                auto &pineMesh = model->parts[0].mesh;
                VertBuffer::uploadSingleMesh(pineMesh);
                variant.lodMeshes.push_back(pineMesh);
                variant.texture = Texture::fromDDSFile("assets/textures/pine_tree.dds");
                variant.buildingMesh = buildingMesh;
            }
        }
        RenderBuilding rb;
        rb.buildingMesh = buildingMesh;
        rb.variant = mu::randomInt(buildingMesh->variants.size() - 1);
        b->renderBuilding = rb;

        float width = mu::random(.8, 1.1);
        b->localTransform = scale(mat4(1), vec3(width, mu::random(.8, 1.1), width));
        b->localTransform = translate(b->localTransform, mu::Y * -mu::random(1.5));
//        b->localTransform = rotate(b->localTransform, mu::random(mu::PI * 2.), mu::Y);

        b->localTransform = rotate(
                b->localTransform,
                mu::random() > .6 ? mu::random(.2) : mu::random(.05),


                rotate(mu::X, mu::random(2 * mu::PI), mu::Y)
        );
    }

    void create_HOUSE(Building &b)
    {
        static BuildingMesh *buildingMesh = NULL;

        std::cout << "create House\n";

        if (!buildingMesh)
        {
            buildingMesh = new BuildingMesh{ DEFAULT };
            auto &variant = buildingMesh->variants.emplace_back();
            {
                SharedModel model = JsonModelLoader::fromUbjsonFile("assets/models/house.ubj", &DEFAULT_BUILDING_VERT_ATTRS)[0];
                auto &pineMesh = model->parts[0].mesh;
                VertBuffer::uploadSingleMesh(pineMesh);
                variant.lodMeshes.push_back(pineMesh);
                variant.texture = Texture::fromDDSFile("assets/textures/house.dds");
                variant.buildingMesh = buildingMesh;
            }
        }
        RenderBuilding rb;
        rb.buildingMesh = buildingMesh;
        rb.variant = mu::randomInt(buildingMesh->variants.size() - 1);
        b->renderBuilding = rb;
    }


    void create_OAK_TREE(Building &b)
    {
        std::cout << "create Oak Tree\n";
    }

}
