#pragma once

#include <VRPG/Mesh/MeshComponent.h>
#include <VRPG/Mesh/SkeletonAnimation.h>

VRPG_MESH_BEGIN

struct Mesh
{
    struct MeshComponentBinding
    {
        std::string   name;
        MeshComponent mesh;

        int  boneIndex = -1; // 为负表示此component没有被绑定到任何骨骼上

        Mat4 nonbindingTransform;
        Mat4 bindingTransform;
    };

    std::vector<MeshComponentBinding>                     staticComponents;
    std::map<std::string, SkeletonAnimation, std::less<>> skeletonAnimations;
    StaticSkeleton                                        staticSkeleton;

    void RemoveUnusedBones();

    void Write(std::ostream &out) const;

    void WriteText(std::ostream &out) const;

    void Read(std::istream &in);

    void ReadText(std::istream &in);
};

void SaveMeshToFile(const Mesh &mesh, const std::string &filename);

Mesh LoadMeshFromFile(const std::string &filename);

VRPG_MESH_END
