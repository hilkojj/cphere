#ifndef BUILDING_H
#define BUILDING_H

#include <optional>
#include <graphics/3d/mesh.h>
#include <graphics/texture.h>

struct Building_;
typedef std::shared_ptr<Building_> Building;

#include "blueprint.h"

static const VertAttributes
    DEFAULT_BUILDING_VERT_ATTRS = VertAttributes()
        .add_(VertAttributes::POSITION)
        .add_(VertAttributes::NORMAL)
        .add_(VertAttributes::TANGENT)
        .add_(VertAttributes::TEX_COORDS),

    BUILDING_TRANSFORM_VERT_ATTRS = VertAttributes()
        .add_(VertAttributes::TRANSFORM_COL_A)
        .add_(VertAttributes::TRANSFORM_COL_B)
        .add_(VertAttributes::TRANSFORM_COL_C)
        .add_(VertAttributes::TRANSFORM_COL_D);


class BuildingRenderingSystem;
class BuildingsSystem;

struct BuildingMeshVariant
{
    std::vector<SharedMesh> lodMeshes; // MUST BE UPLOADED TO SAME VertBuffer! lodMeshes[0] should have most detail
//    std::vector<SharedMesh> lodShadowMeshes; // must be uploaded to same VertBuffer as above.

    SharedTexture texture;
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

    Building_(Blueprint *bp, int x, int y, int rotation, Island *isl);

  private:
    friend BuildingsSystem;
    /**
     * DONT CALL THIS FUNCTION AFTER PLACING THE BUILDING!
     */
    void move(int x, int y, int rotation, Island *isl);
};

#endif
