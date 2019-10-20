#ifndef BUILDING_H
#define BUILDING_H

#include <optional>
#include <graphics/3d/mesh.h>
#include <graphics/texture.h>

struct Building_;
typedef std::shared_ptr<Building_> Building;

#include "blueprint.h"

static const VertAttributes defaultBuildingVertAttrs = VertAttributes()
        .add_(VertAttributes::POSITION)
        .add_(VertAttributes::NORMAL)
        .add_(VertAttributes::TANGENT)
        .add_(VertAttributes::TEX_COORDS);

class BuildingRenderingSystem;
class BuildingsSystem;

struct BuildingMeshVariant
{
    std::vector<SharedMesh> lodMeshes; // MUST BE UPLOADED TO SAME VertBuffer! lodMeshes[0] should have most detail
//    std::vector<SharedMesh> lodShadowMeshes; // must be uploaded to same VertBuffer as above.

    SharedTexture texture;

private:
    friend BuildingRenderingSystem;

    int instancedVertDataId = -1;
    VertData instanceTransforms = VertData(VertAttributes().add_(VertAttributes::TRANSFORM), std::vector<float>());
    std::vector<Building>
        instances,
        instancesToAdd,
        instancesToRemove;
};

struct BuildingMesh
{
    std::vector<BuildingMeshVariant> variants;
    std::optional<ShaderProgram> customShaderProgram;
};

struct RenderBuilding
{
    BuildingMesh *buildingMesh;
    unsigned int variant;

private:
    int instanceIndex = -1; // remember the index so that the instance-data can be removed from BuildingMeshVariant.instanceTransforms
    friend BuildingRenderingSystem;
};

class Island;

struct Building_
{
    Blueprint *bp;
    Island *isl;
    int rotation; // how many times the building is rotated (90 degrees)
    std::vector<ivec2> tiles; // the tiles occupied by the building
    mat4 transform;

    std::optional<RenderBuilding> renderBuilding;

    Building_(Blueprint *bp, int x, int y, int rotation, Island *isl)

        : bp(bp), tiles(bp->width * bp->height)
    {
        move(x, y, rotation, isl);
    }

  private:
    friend BuildingsSystem;
    /**
     * DONT CALL THIS FUNCTION AFTER PLACING THE BUILDING!
     */
    void move(int x, int y, int rotation, Island *isl)
    {
        int i = 0;
        bool rot = rotation % 2 == 0;
        for (int x0 = 0; x0 < bp->width; x0++)
            for (int y0 = 0; y0 < bp->height; y0++)
                tiles[i++] = ivec2(x + (rot ? x0 : y0), y + (rot ? y0 : x0));

        this->rotation = rotation;
        this->isl = isl;
        if (isl)
        {
            // todo: calculate transform:
            this->transform = mat4(1);
        }
    }
};

#endif
