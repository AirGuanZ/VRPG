#pragma once

#include <vector>

#include <VRPG/Mesh/Common.h>

VRPG_MESH_BEGIN

/**
 * @brief 静态骨架
 */
class StaticSkeleton
{
public:

    struct Bone
    {
        int parent = -1;
        Mat4 staticTransform;
        std::string name;
    };

    explicit StaticSkeleton(std::vector<Bone> bones = {});

    int GetBoneCount() const noexcept;

    const Bone &GetBone(int boneIndex) const noexcept;

    const Bone &operator[](int boneIndex) const noexcept;

    int GetParentIndex(int boneIndex) const noexcept;
    const Mat4 &GetTransform(int boneIndex) const noexcept;

    int BoneNameToIndex(std::string_view boneName) const;

    void Write(std::ostream &out) const;

    void Read(std::istream &in);

private:

    std::vector<Bone> bones_;
    std::map<std::string, int, std::less<>> name2Index_;
};

VRPG_MESH_END
