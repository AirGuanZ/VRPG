#include <fstream>
#include <type_traits>

#include <VRPG/Mesh/Mesh.h>
#include <VRPG/Mesh/Utility.h>

VRPG_MESH_BEGIN

void Mesh::Write(std::ostream &out) const
{
    uint32_t bindingCount = static_cast<uint32_t>(staticComponents.size());
    WriteOStream(out, bindingCount);
    for(auto &binding : staticComponents)
    {
        WriteOStream(out, binding.name);
        binding.mesh.Write(out);
        WriteOStream(out, binding.boneName);
        WriteOStream(out, binding.bindingTransform);
    }

    uint32_t animationCount = static_cast<uint32_t>(skeletonAnimation.size());
    WriteOStream(out, animationCount);
    for(auto &p : skeletonAnimation)
    {
        WriteOStream(out, p.first);
        p.second.Write(out);
    }

    staticSkeleton.Write(out);
}

void Mesh::Read(std::istream &in)
{
    staticComponents.clear();
    skeletonAnimation.clear();

    uint32_t bindingCount;
    ReadIStream(in, bindingCount);
    staticComponents.resize(bindingCount);
    for(auto &comp : staticComponents)
    {
        ReadIStream(in, comp.name);
        comp.mesh.Read(in);
        ReadIStream(in, comp.boneName);
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

        skeletonAnimation.insert(std::make_pair(std::move(name), std::move(ani)));
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
