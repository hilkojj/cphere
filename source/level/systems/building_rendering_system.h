#ifndef GAME_BUILDING_RENDERING_H
#define GAME_BUILDING_RENDERING_H

#include <optional>

#include "../level.h"
#include "graphics/texture.h"
#include "../buildings/building.h"
#include "buildings_system.h"

class BuildingRenderingSystem : public LevelSystem
{
  public:
    inline static BuildingRenderingSystem *active = NULL;

    ShaderProgram defaultShader;
    std::map<Island*, std::vector<BuildingMesh*>> buildingMeshes;

    BuildingRenderingSystem(Level *lvl)
        : defaultShader(
                ShaderProgram::fromFiles("DefaultBuildingShader", "assets/shaders/building.vert", "assets/shaders/building.frag")
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

            rb.buildingMesh->variants[rb.variant].instancesToAdd.push_back(b);
        });


        BuildingsSystem::active->onDestroyed.emplace_back([&](Building b) {

            if (!b->renderBuilding) return;
            std::cout << "bye\n";
            RenderBuilding &rb = b->renderBuilding.value();
            if (rb.instanceIndex == -1) return;

            auto &var = rb.buildingMesh->variants[rb.variant];
            var.instancesToRemove.push_back(b);
        });
    }

    void update(double deltaTime, Level *lvl) override
    {}

    void render(double deltaTime, Level *lvl, vec3 &sunDir)
    {
        defaultShader.use();
        glUniformMatrix4fv(defaultShader.location("view"), 1, GL_FALSE, &lvl->cam->combined[0][0]);
        glUniform3f(defaultShader.location("sunDir"), sunDir.x, sunDir.y, sunDir.z);

        for (Island *isl : lvl->earth.islands)
        {
            if (!isl->isInView || !buildingMeshes.count(isl)) continue;

            for (auto &buildingMesh : buildingMeshes[isl])
            {
                auto vertBuffer = buildingMesh->variants[0].lodMeshes[0]->vertBuffer;
                for (auto &var : buildingMesh->variants)
                {
                    auto nrToAdd = var.instancesToAdd.size(), nrToRemove = var.instancesToRemove.size();
                    if (nrToAdd || nrToRemove)
                    {
                        if (var.instancedVertDataId != -1)
                            vertBuffer->deletePerInstanceData(var.instancedVertDataId);

                        for (auto &b : var.instancesToRemove)
                        {
                            var.instanceTransforms.setMat<mat4>(
                                    var.instanceTransforms.getMat<mat4>(var.instances.size() - 1, 0),
                                    b->renderBuilding->instanceIndex, 0
                            );
                            var.instanceTransforms.removeVertices(1);

                            // remove instance from list:
                            var.instances.erase(std::remove(var.instances.begin(), var.instances.end(), b), var.instances.end());
                        }

                        for (auto &b : var.instancesToAdd)
                        {
                            b->renderBuilding->instanceIndex = var.instances.size();
                            var.instanceTransforms.addVertices(1);
                            var.instanceTransforms.setMat<mat4>(b->transform, var.instances.size(), 0);
                            var.instances.push_back(b);
                        }

                        var.instancedVertDataId = vertBuffer->uploadPerInstanceData(var.instanceTransforms);
                        var.instancesToAdd.clear();
                        var.instancesToRemove.clear();
                        std::cout << var.instances.size() << "\n";
                    }
                    vertBuffer->usePerInstanceData(var.instancedVertDataId);
                    var.texture->bind(0, defaultShader, "buildingTexture");
                    var.lodMeshes[0]->renderInstances(var.instances.size());
                }
            }
        }
    }

};

#endif
