#include <fstream>
#include <queue>
#include <type_traits>

#include <VRPG/Mesh/Mesh.h>
#include <VRPG/Mesh/Utility.h>

VRPG_MESH_BEGIN

void Mesh::RemoveUnusedBones()
{
    // 构建骨骼i的名字到i的映射

    std::map<std::string, int> nameToIndex;
    for(int i = 0; i < staticSkeleton.GetBoneCount(); ++i)
    {
        auto &bone = staticSkeleton.GetBone(i);
        nameToIndex.insert(std::make_pair(bone.name, i));
    }

    // 计算骨骼i是否被使用了
    // 被使用的标准：
    // 1. 有mesh component绑定到这上面，或
    // 2. 有孩子被使用了

    std::vector<bool> isBoneUsed(staticSkeleton.GetBoneCount());

    std::queue<int> usedBoneQueue;
    for(auto &binding : staticComponents)
    {
        if(binding.boneIndex >= 0)
        {
            isBoneUsed[binding.boneIndex] = true;
            usedBoneQueue.push(binding.boneIndex);
        }
    }

    while(!usedBoneQueue.empty())
    {
        int boneIndex = usedBoneQueue.front();
        usedBoneQueue.pop();
        int parentIndex = staticSkeleton.GetParentIndex(boneIndex);
        if(parentIndex >= 0)
        {
            isBoneUsed[parentIndex] = true;
            usedBoneQueue.push(parentIndex);
        }
    }

    // 将用了的骨骼整理到一起，构建 新i->旧i 以及 旧i -> 新i 两个映射

    std::vector<int> newIndexToOldIndex;
    std::vector<int> oldIndexToNewIndex(staticSkeleton.GetBoneCount(), -1);

    for(int i = 0; i < staticSkeleton.GetBoneCount(); ++i)
    {
        if(isBoneUsed[i])
        {
            int newIndex = static_cast<int>(newIndexToOldIndex.size());
            newIndexToOldIndex.push_back(i);
            oldIndexToNewIndex[i] = newIndex;
        }
    }

    // 重新计算仅由新index构成的static skeleton

    std::vector<StaticSkeleton::Bone> newStaticSkeletonBones(newIndexToOldIndex.size());

    int newIndexCount = static_cast<int>(newIndexToOldIndex.size());
    for(int newIndex = 0; newIndex < newIndexCount; ++newIndex)
    {
        int oldIndex = newIndexToOldIndex[newIndex];
        auto &newBone = newStaticSkeletonBones[newIndex];
        auto &oldBone = staticSkeleton.GetBone(oldIndex);

        newBone.name   = oldBone.name;

        int oldParentIndex = oldBone.parent;
        if(oldParentIndex >= 0)
        {
            newBone.parent = oldIndexToNewIndex[oldParentIndex];
        }

        newBone.staticTransform = oldBone.staticTransform;
    }

    // 重新计算仅由新index构成的skeleton animation

    std::map<std::string, SkeletonAnimation, std::less<>> newSkeletonAnimations;
    for(auto &p : skeletonAnimations)
    {
        auto &animationName = p.first;
        auto &animation     = p.second;

        std::vector<BoneAnimation> newBoneAnimations(newIndexToOldIndex.size());
        for(int newIndex = 0; newIndex < newIndexCount; ++newIndex)
        {
            int oldIndex = newIndexToOldIndex[newIndex];
            newBoneAnimations[newIndex] = animation.GetBoneAnimation(oldIndex);
        }

        newSkeletonAnimations.insert(std::make_pair(
            animationName, SkeletonAnimation(std::move(newBoneAnimations))));
    }

    // 重新构建mesh components binding

    for(auto &binding : staticComponents)
    {
        if(binding.boneIndex >= 0)
        {
            binding.boneIndex = oldIndexToNewIndex[binding.boneIndex];
        }
    }

    // happy ending!

    staticSkeleton     = StaticSkeleton(std::move(newStaticSkeletonBones));
    skeletonAnimations = std::move(newSkeletonAnimations);
}

void Mesh::Write(std::ostream &out) const
{
    uint32_t bindingCount = static_cast<uint32_t>(staticComponents.size());
    WriteOStream(out, bindingCount);
    for(auto &binding : staticComponents)
    {
        WriteOStream(out, binding.name);
        binding.mesh.Write(out);
        WriteOStream(out, binding.boneIndex);
        WriteOStream(out, binding.bindingTransform);
    }

    uint32_t animationCount = static_cast<uint32_t>(skeletonAnimations.size());
    WriteOStream(out, animationCount);
    for(auto &p : skeletonAnimations)
    {
        WriteOStream(out, p.first);
        p.second.Write(out);
    }

    staticSkeleton.Write(out);
}

void Mesh::Read(std::istream &in)
{
    staticComponents.clear();
    skeletonAnimations.clear();

    uint32_t bindingCount;
    ReadIStream(in, bindingCount);
    staticComponents.resize(bindingCount);
    for(auto &comp : staticComponents)
    {
        ReadIStream(in, comp.name);
        comp.mesh.Read(in);
        ReadIStream(in, comp.boneIndex);
        ReadIStream(in, comp.bindingTransform);
    }

    uint32_t animationCount;
    ReadIStream(in, animationCount);
    for(size_t i = 0; i < animationCount; ++i)
    {
        std::string name;
        ReadIStream(in, name);

        SkeletonAnimation ani({});
        ani.Read(in);

        skeletonAnimations.insert(std::make_pair(std::move(name), std::move(ani)));
    }

    staticSkeleton.Read(in);
}

void SaveMeshToFile(const Mesh &mesh, const std::string &filename)
{
    std::ofstream fout(filename, std::ios::out | std::ios::binary | std::ios::trunc);
    if(!fout)
    {
        throw VRPGMeshException("failed to open file: " + filename);
    }
    mesh.Write(fout);
}

Mesh LoadMeshFromFile(const std::string &filename)
{
    std::ifstream fin(filename, std::ios::in | std::ios::binary);
    if(!fin)
    {
        throw VRPGMeshException("failed to open file: " + filename);
    }

    Mesh mesh;
    mesh.Read(fin);

    return mesh;
}

VRPG_MESH_END
