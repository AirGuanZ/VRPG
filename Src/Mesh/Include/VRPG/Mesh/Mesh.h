#pragma once

#include <VRPG/Mesh/MeshComponent.h>
#include <VRPG/Mesh/SkeletonAnimation.h>

VRPG_MESH_BEGIN

struct Mesh
{
    struct MeshComponentBinding
    {
        std::string name;
        MeshComponent mesh;

        std::string boneName;
        Mat4 bindingTransform;
    };

    std::vector<MeshComponentBinding>        staticComponents;
    std::map<std::string, SkeletonAnimation> skeletonAnimation;
    StaticSkeleton                           staticSkeleton;

    void Write(std::ostream &out) const;

    void Read(std::istream &in);
};

void SaveMeshToFile(const Mesh &mesh, const std::string &filename);

Mesh LoadMeshFromFile(const std::string &filename);

VRPG_MESH_END
