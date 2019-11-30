#include <fstream>

#include <VRPG/Game/Mesh/DiffuseSolidMesh.h>

VRPG_GAME_BEGIN

std::unique_ptr<DiffuseSolidMesh> DiffuseSolidMesh::LoadFromConfig(
    std::shared_ptr<const DiffuseSolidMeshEffect> effect, const libconfig::Setting &config)
{
    LoadParams loadParams;

    loadParams.effect = std::move(effect);

    if(!config.lookupValue("Filename", loadParams.meshFilename))
    {
        throw VRPGGameException("mesh filename not found in creating diffuse solid mesh from config");
    }

    auto &diffuseTextureConfig = config.lookup("DiffuseTexture");
    for(auto &s : diffuseTextureConfig)
    {
        std::string name     = s.getName();
        std::string filename = s.c_str();
        loadParams.diffuseTextureFilenames.insert(std::make_pair(name, filename));
    }

    return LoadFromFile(loadParams);
}

std::unique_ptr<DiffuseSolidMesh> DiffuseSolidMesh::LoadFromFile(const LoadParams &params)
{
    std::ifstream fin(params.meshFilename, std::ios::in | std::ios::binary);
    if(!fin)
    {
        throw VRPGGameException("failed to open file: " + params.meshFilename);
    }

    Mesh::Mesh mesh;
    mesh.Read(fin);
    mesh.RemoveUnusedBones();

    fin.close();

    std::map<std::string, ComPtr<ID3D11ShaderResourceView>> diffuseTextures;
    for(auto &p : params.diffuseTextureFilenames)
    {
        auto srv = Base::D3D::Texture2DLoader(2.2f).LoadFromFile(p.second);
        diffuseTextures.insert(std::make_pair(p.first, std::move(srv)));
    }

    auto ptr = new DiffuseSolidMesh(mesh, params.effect, std::move(diffuseTextures));
    return std::unique_ptr<DiffuseSolidMesh>(ptr);
}

std::unique_ptr<DiffuseSolidMesh> DiffuseSolidMesh::Clone() const
{
    auto ptr = new DiffuseSolidMesh(
        worldMatrix_, brightness_, model_,
        animationState_, currentGlobalTransforms_);
    return std::unique_ptr<DiffuseSolidMesh>(ptr);
}

const std::shared_ptr<const DiffuseSolidMeshEffect> &DiffuseSolidMesh::GetEffect() const noexcept
{
    return model_->effect;
}

const Mesh::SkeletonAnimation *DiffuseSolidMesh::GetCurrentAnimation() const noexcept
{
    return animationState_.GetCurrentAnimation();
}

void DiffuseSolidMesh::SetCurrentAnimation(std::string_view animationName)
{
    animationState_.SetCurrentAnimationName(animationName);
}

bool DiffuseSolidMesh::IsAnimationLoopEnabled() const noexcept
{
    return animationState_.IsAnimationLoopEnabled();
}

void DiffuseSolidMesh::EnableAnimationLoop(bool loop)
{
    animationState_.EnableAnimationLoop(loop);
}

bool DiffuseSolidMesh::IsAnimationEnd() const noexcept
{
    return animationState_.IsAnimationEnd();
}

float DiffuseSolidMesh::GetCurrentAnimationTime() const noexcept
{
    return animationState_.GetCurrentAnimationTime();
}

void DiffuseSolidMesh::SetCurrentAnimationTime(float timePoint)
{
    animationState_.SetCurrentAnimationTime(timePoint);
}

void DiffuseSolidMesh::RenderForward(const ForwardRenderParams &params) const
{
    const Mat4 &viewProj = params.camera->GetViewProjectionMatrix();

    for(auto &mesh : model_->meshComponents)
    {
        Mat4 world;
        if(int boneIndex = mesh.boneIndex; boneIndex >= 0)
        {
            world = mesh.bindingTransform * currentGlobalTransforms_[boneIndex] * worldMatrix_;
        }
        else
        {
            world = mesh.nonbindingTransform * worldMatrix_;
        }
        Mat4 worldViewProj = world * viewProj;

        model_->effect->BindForwardVSTransform(world, worldViewProj);
        model_->effect->BindForwardPSBrightness(brightness_);
        model_->effect->BindForwardPSDiffuseTexture(mesh.diffuseTexture);

        mesh.submesh->Render();
    }
}

void DiffuseSolidMesh::RenderShadow(const ShadowRenderParams &params) const
{
    for(auto &mesh : model_->meshComponents)
    {
        Mat4 world;
        if(int boneIndex = mesh.boneIndex; boneIndex >= 0)
        {
            world = mesh.bindingTransform * currentGlobalTransforms_[boneIndex] * worldMatrix_;
        }
        else
        {
            world = mesh.nonbindingTransform * worldMatrix_;
        }
        Mat4 worldViewProj = world * params.shadowViewProj;

        model_->effect->BindShadowVSTransform(worldViewProj);

        mesh.submesh->Render();
    }
}

void DiffuseSolidMesh::SetWorldTransform(const Mat4 &worldMatrix)
{
    worldMatrix_ = worldMatrix;
}

void DiffuseSolidMesh::SetBrightness(const Vec4 &brightness)
{
    brightness_ = brightness;
}

void DiffuseSolidMesh::UpdateBoneTransform()
{
    animationState_.ComputeTransform(currentGlobalTransforms_.data());
}

DiffuseSolidMesh::DiffuseSolidMesh(
    const Mesh::Mesh &mesh, std::shared_ptr<const DiffuseSolidMeshEffect> effect,
    std::map<std::string, ComPtr<ID3D11ShaderResourceView>> diffuseTextures)
{
    worldMatrix_ = Mat4::identity();

    model_ = std::make_shared<Model>();
    model_->effect         = std::move(effect);
    model_->animationModel = std::make_shared<AnimationModel>(mesh.staticSkeleton, mesh.skeletonAnimations);

    model_->meshComponents.reserve(mesh.staticComponents.size());
    for(auto &m : mesh.staticComponents)
    {
        MeshBinding component;

        component.submesh = std::make_unique<Submesh>();
        InitializeSubmesh(m.mesh, *component.submesh);

        component.nonbindingTransform = m.nonbindingTransform;
        component.bindingTransform    = m.bindingTransform;

        auto it = diffuseTextures.find(m.name);
        if(it == diffuseTextures.end())
        {
            auto itDefault = diffuseTextures.find("Default");
            if(itDefault == diffuseTextures.end())
            {
                throw VRPGGameException("Default texture not found for submesh: " + m.name);
            }
            component.diffuseTexture = itDefault->second;
        }
        else
        {
            component.diffuseTexture = it->second;
        }

        component.boneIndex = m.boneIndex;

        model_->meshComponents.push_back(std::move(component));
    }

    animationState_.Initialize(model_->animationModel);
    currentGlobalTransforms_.resize(model_->animationModel->GetBoneCount(), Mat4::identity());
}

DiffuseSolidMesh::DiffuseSolidMesh(
    const Mat4 &worldMatrix, const Vec4 &brightness, std::shared_ptr<Model> model,
    AnimationState animationState, std::vector<Mat4> currentGlobalTransforms)
{
    worldMatrix_ = worldMatrix;
    brightness_  = brightness;

    model_  = std::move(model);

    animationState_          = std::move(animationState);
    currentGlobalTransforms_ = std::move(currentGlobalTransforms);
}

void DiffuseSolidMesh::InitializeSubmesh(const Mesh::MeshComponent &component, Submesh &submesh)
{
    std::vector<Vertex> vertexData;
    std::vector<Index>  indexData;

    vertexData.reserve(component.vertices.size());
    for(auto &v : component.vertices)
    {
        Vertex newVertex;
        newVertex.position = v.position;
        newVertex.normal   = v.normal;
        newVertex.texCoord = v.uv;
        vertexData.push_back(newVertex);
    }

    indexData.reserve(component.indices.size());
    for(auto i : component.indices)
    {
        indexData.push_back(static_cast<Index>(i));
    }

    submesh.Initialize(
        vertexData.data(), static_cast<UINT>(vertexData.size()),
        indexData.data(),   static_cast<UINT>(indexData.size()));
}

VRPG_GAME_END
