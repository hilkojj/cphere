#ifndef GAME_BUILDING_RENDERING_H
#define GAME_BUILDING_RENDERING_H

#include <optional>
#include <graphics/orthographic_camera.h>

#include "../level.h"
#include "graphics/texture.h"
#include "../buildings/building.h"
#include "buildings_system.h"

struct VariantInstances
{
    int vertDataId = -1;
    VertData transforms = VertData(BUILDING_TRANSFORM_VERT_ATTRS, std::vector<float>());
    std::vector<Building>
            buildings,
            toAdd,
            toRemove;
};

class BuildingRenderingSystem : public LevelSystem
{
  public:
    inline static BuildingRenderingSystem *active = NULL;

    ShaderProgram defaultShader, treeShader;
    std::map<Island*, std::vector<BuildingMesh*>> buildingMeshes;
    std::map<Island*, std::map<BuildingMeshVariant*, VariantInstances>> variantInstances;

    BuildingRenderingSystem(Level *lvl)
        : defaultShader(
                ShaderProgram::fromFiles("DefaultBuildingShader", "assets/shaders/building.vert", "assets/shaders/building.frag")
          ),
          treeShader(
                ShaderProgram::fromFiles("TreeShader", "assets/shaders/building.vert", "assets/shaders/tree.frag")
          )
    {
        active = this;


        BuildingsSystem::active->onPlaced.emplace_back([&](Building b) {

            if (!b->renderBuilding) return;
            RenderBuilding &rb = b->renderBuilding.value();

            std::cout << "Add building to instanced rendering on next render...\n";

            auto &islMeshes = buildingMeshes[b->isl];
            if (std::find(islMeshes.begin(), islMeshes.end(), rb.buildingMesh) == islMeshes.end())
                islMeshes.push_back(rb.buildingMesh);

            auto &var = rb.buildingMesh->variants[rb.variant];
            variantInstances[b->isl][&var].toAdd.push_back(b);
        });


        BuildingsSystem::active->onDestroyed.emplace_back([&](Building b) {

            if (!b->renderBuilding) return;
            std::cout << "bye\n";
            RenderBuilding &rb = b->renderBuilding.value();
            if (rb.instanceIndex == -1) return;

            auto &var = rb.buildingMesh->variants[rb.variant];
            variantInstances[b->isl][&var].toRemove.push_back(b);
        });
    }

    void update(double deltaTime, Level *lvl) override
    {}

    void render(double deltaTime, Level *lvl, vec3 &sunDir)
    {
        for (Island *isl : lvl->earth.islands)
        {
            if (!isl->isInView) continue;

            for (auto &[var, instances] : variantInstances[isl])
            {
                ShaderProgram *shader = &defaultShader;
                switch (var->buildingMesh->shader)
                {
                    case TREE:
                        shader = &treeShader;
                        break;
                }
                shader->use();

                glUniformMatrix4fv(shader->location("view"), 1, GL_FALSE, &lvl->cam->combined[0][0]);
                glUniform3f(shader->location("sunDir"), sunDir.x, sunDir.y, sunDir.z);

                auto vertBuffer = var->lodMeshes[0]->vertBuffer;
                auto nrToAdd = instances.toAdd.size(), nrToRemove = instances.toRemove.size();
                if (nrToAdd || nrToRemove)
                {
                    if (instances.vertDataId != -1)
                        vertBuffer->deletePerInstanceData(instances.vertDataId);

                    for (auto &b : instances.toRemove)
                    {
                        instances.transforms.setMat<mat4>(
                                instances.transforms.getMat<mat4>(instances.buildings.size() - 1, 0),
                                b->renderBuilding->instanceIndex, 0
                        );
                        instances.transforms.setFloat(
                                instances.transforms.getFloat(instances.buildings.size() - 1, 16),
                                b->renderBuilding->instanceIndex, 16
                        );
                        instances.transforms.removeVertices(1);

                        // remove instance from list:
                        instances.buildings.erase(std::remove(instances.buildings.begin(), instances.buildings.end(), b), instances.buildings.end());
                    }

                    for (auto &b : instances.toAdd)
                    {
                        b->renderBuilding->instanceIndex = instances.buildings.size();
                        instances.transforms.addVertices(1);
                        instances.transforms.setMat<mat4>(b->transform, instances.buildings.size(), 0);
                        instances.transforms.setFloat(mu::random(), instances.buildings.size(), 16);
                        instances.buildings.push_back(b);
                    }

                    instances.vertDataId = vertBuffer->uploadPerInstanceData(instances.transforms);
                    instances.toAdd.clear();
                    instances.toRemove.clear();
                    std::cout << instances.buildings.size() << "\n";
                }
                vertBuffer->usePerInstanceData(instances.vertDataId);
                var->texture->bind(0);
                glUniform1i(shader->location("buildingTexture"), 0);
                var->lodMeshes[0]->renderInstances(instances.buildings.size());
            }
        }
    }

    void renderShadows(Level *lvl, OrthographicCamera *sunCam)
    {
        defaultShader.use();
        glUniformMatrix4fv(defaultShader.location("view"), 1, GL_FALSE, &sunCam->combined[0][0]);

        for (Island *isl : lvl->earth.islands)
        {
            if (!isl->isInView) continue;

            for (auto &[var, instances] : variantInstances[isl])
            {
                if (instances.vertDataId == -1) continue;

                auto vertBuffer = var->lodMeshes[0]->vertBuffer;

                vertBuffer->usePerInstanceData(instances.vertDataId);
                var->texture->bind(0, defaultShader, "buildingTexture");
                var->lodMeshes[0]->renderInstances(instances.buildings.size());
            }
        }
    }

};

#endif
