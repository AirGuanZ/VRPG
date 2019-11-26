#include <limits>

#include <VRPG/Mesh/StaticSkeleton.h>
#include <VRPG/Mesh/Utility.h>

VRPG_MESH_BEGIN

StaticSkeleton::StaticSkeleton(std::vector<Bone> bones)
    : bones_(std::move(bones))
{
    for(size_t i = 0; i < bones_.size(); ++i)
    {
        auto &bone = bones_[i];
        if(name2Index_.find(bone.name) != name2Index_.end())
        {
            throw std::runtime_error("repeated bone name: " + bone.name);
        }
        name2Index_[bone.name] = static_cast<int>(i);
    }
}

int StaticSkeleton::GetBoneCount() const noexcept
{
    return static_cast<int>(bones_.size());
}

const StaticSkeleton::Bone &StaticSkeleton::GetBone(int boneIndex) const noexcept
{
    return bones_[boneIndex];
}

const StaticSkeleton::Bone &StaticSkeleton::operator[](int boneIndex) const noexcept
{
    return GetBone(boneIndex);
}

int StaticSkeleton::GetParentIndex(int boneIndex) const noexcept
{
    return GetBone(boneIndex).parent;
}

const Mat4 &StaticSkeleton::GetTransform(int boneIndex) const noexcept
{
    return GetBone(boneIndex).staticTransform;
}

int StaticSkeleton::BoneNameToIndex(std::string_view boneName) const
{
    auto it = name2Index_.find(boneName);
    return it != name2Index_.end() ? it->second : -1;
}

void StaticSkeleton::ComputeStaticTransformMatrix(Mat4 *output) const
{
    int boneCount = GetBoneCount();
    for(int i = 0; i < boneCount; ++i)
    {
        auto &bone = GetBone(i);
        if(bone.parent < 0)
        {
            output[i] = bone.staticTransform;
        }
        else
        {
            output[i] = bone.staticTransform * output[bone.parent];
        }
    }
}

void StaticSkeleton::Write(std::ostream &out) const
{
    uint32_t boneCount = static_cast<uint32_t>(bones_.size());
    WriteOStream(out, boneCount);
    for(auto &bone : bones_)
    {
        WriteOStream(out, bone.parent);
        WriteOStream(out, bone.staticTransform);
        WriteOStream(out, bone.name);
    }

    uint32_t nameCount = static_cast<uint32_t>(name2Index_.size());
    WriteOStream(out, nameCount);
    for(auto &p : name2Index_)
    {
        WriteOStream(out, p.first);
        WriteOStream(out, p.second);
    }
}

void StaticSkeleton::Read(std::istream &in)
{
    bones_.clear();
    name2Index_.clear();

    uint32_t boneCount;
    ReadIStream(in, boneCount);

    bones_.resize(boneCount);
    for(auto &bone : bones_)
    {
        ReadIStream(in, bone.parent);
        ReadIStream(in, bone.staticTransform);
        ReadIStream(in, bone.name);
    }

    uint32_t nameCount;
    ReadIStream(in, nameCount);
    for(uint32_t i = 0; i < nameCount; ++i)
    {
        std::string name;
        ReadIStream(in, name);

        int index;
        ReadIStream(in, index);

        name2Index_.insert(std::make_pair(std::move(name), index));
    }
}

VRPG_MESH_END
