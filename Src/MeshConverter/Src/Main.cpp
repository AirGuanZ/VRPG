#include <iostream>

#include <agz/utility/misc.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <Misc/cxxopts.hpp>

#include <VRPG/Mesh/Mesh.h>
#include <VRPG/MeshConverter/MeshLoader.h>

struct ConverterParams
{
    std::string inputFilename;
    std::string outputFilename;
};

ConverterParams ParseParams(int argc, char *argv[])
{
    cxxopts::Options options("MeshConverter");
    options.add_options("")
        ("i,input", "input filename", cxxopts::value<std::string>())
        ("o,output", "output filename", cxxopts::value<std::string>());
    auto parseResult = options.parse(argc, argv);

    ConverterParams params;
    params.inputFilename  = parseResult["input"].as<std::string>();
    params.outputFilename = parseResult["output"].as<std::string>();

    return params;
}

void Run(int argc, char *argv[])
{
    auto params = ParseParams(argc, argv);

    constexpr unsigned ASSIMP_IMPORT_FLAG =
        aiProcess_Triangulate    |
        aiProcess_GenNormals     |
        aiProcess_FlipUVs        |
        aiProcess_MakeLeftHanded |
        aiProcess_FlipWindingOrder;

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        params.inputFilename, ASSIMP_IMPORT_FLAG);
    if(!scene)
    {
        throw VRPG::MeshConverter::VRPGMeshConverterException(importer.GetErrorString());
    }

    auto mesh = VRPG::MeshConverter::LoadMesh(scene);
    SaveMeshToFile(mesh, params.outputFilename);
}

int main(int argc, char *argv[])
{
    try
    {
        spdlog::set_pattern("%v");
        Run(argc, argv);
    }
    catch(const std::exception & err)
    {
        agz::misc::extract_hierarchy_exceptions(
            err, std::ostream_iterator<std::string>(std::cout, "\n"));
        return -1;
    }
    catch(...)
    {
        std::cout << "an unknown error occurred" << std::endl;
    }
}
