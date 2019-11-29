#include <fstream>

#include <VRPG/Game/AnimatedMesh/DiffuseSolidMesh.h>

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

    fin.close();

    std::map<std::string, ComPtr<ID3D11ShaderResourceView>> diffuseTextures;
    for(auto &p : params.diffuseTextureFilenames)
    {
        auto srv = Base::D3D::Texture2DLoader(2.2f).LoadFromFile(p.second);
        diffuseTextures.insert(std::make_pair(p.first, std::move(srv)));
    }

    auto ptr = new DiffuseSolidMesh(params.effect, mesh, std::move(diffuseTextures));
    return std::unique_ptr<DiffuseSolidMesh>(ptr);
}

std::unique_ptr<DiffuseSolidMesh> DiffuseSolidMesh::Clone() const
{
    auto ptr = new DiffuseSolidMesh(
        worldMatrix_, brightness_, effect_, model_,
        currentAnimation_, animationLoop_,
        currentGlobalTransforms_, currentAnimationTime_);
    return std::unique_ptr<DiffuseSolidMesh>(ptr);
}

const std::shared_ptr<const DiffuseSolidMeshEffect> &DiffuseSolidMesh::GetEffect() const noexcept
{
    return effect_;
}

const Mesh::SkeletonAnimation *DiffuseSolidMesh::GetCurrentAnimation() const noexcept
{
    return currentAnimation_;
}

void DiffuseSolidMesh::SetCurrentAnimation(std::string_view animationName)
{
    if(animationName.empty())
    {
        currentAnimation_ = nullptr;
        currentAnimationTime_ = 0;
        return;
    }

    auto it = model_->skeletonAnimations.find(animationName);
    if(it == model_->skeletonAnimations.end())
    {
        throw VRPGGameException("animation not found: " + std::string(animationName));
    }
    currentAnimation_ = &it->second;
    currentAnimationTime_ = 0;
}

bool DiffuseSolidMesh::IsAnimationLoopEnabled() const noexcept
{
    return animationLoop_;
}

void DiffuseSolidMesh::EnableAnimationLoop(bool loop)
{
    animationLoop_ = loop;
}

bool DiffuseSolidMesh::IsAnimationEnd() const noexcept
{
    if(!currentAnimation_)
    {
        return false;
    }
    return currentAnimationTime_ > currentAnimation_->GetEndTime();
}

float DiffuseSolidMesh::GetCurrentAnimationTime() const noexcept
{
    return currentAnimationTime_;
}

void DiffuseSolidMesh::SetCurrentAnimationTime(float timePoint)
{
    if(!currentAnimation_)
    {
        return;
    }

    currentAnimationTime_ = timePoint;
    float endTime = currentAnimation_->GetEndTime();
    if(currentAnimationTime_ > endTime)
    {
        if(animationLoop_)
        {
            while(currentAnimationTime_ > endTime)
            {
                currentAnimationTime_ -= endTime;
            }
        }
        else
        {
            currentAnimationTime_ = endTime;
        }
    }
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

        effect_->BindForwardVSTransform(world, worldViewProj);
        effect_->BindForwardPSBrightness(brightness_);
        effect_->BindForwardPSDiffuseTexture(mesh.diffuseTexture);

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

        effect_->BindShadowVSTransform(worldViewProj);

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
    if(!currentAnimation_)
    {
        model_->staticSkeleton.ComputeStaticTransformMatrix(currentGlobalTransforms_.data());
        return;
    }
    currentAnimation_->ComputeTransformMatrix(model_->staticSkeleton, currentAnimationTime_, currentGlobalTransforms_.data());
}

DiffuseSolidMesh::DiffuseSolidMesh(
    std::shared_ptr<const DiffuseSolidMeshEffect> effect,
    const Mesh::Mesh &mesh,
    std::map<std::string, ComPtr<ID3D11ShaderResourceView>> diffuseTextures,
    std::string_view initAnimationName)
{
    worldMatrix_ = Mat4::identity();

    effect_ = std::move(effect);

    model_ = std::make_shared<Model>();
    model_->staticSkeleton = mesh.staticSkeleton;
    model_->skeletonAnimations = mesh.skeletonAnimations;

    model_->meshComponents.reserve(mesh.staticComponents.size());
    for(auto &m : mesh.staticComponents)
    {
        MeshComponent component;

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

    animationLoop_ = false;
    if(initAnimationName.empty())
    {
        currentAnimation_ = nullptr;
    }
    else
    {
        auto it = model_->skeletonAnimations.find(initAnimationName);
        if(it == model_->skeletonAnimations.end())
        {
            throw VRPGGameException("animation not found: " + std::string(initAnimationName));
        }
        currentAnimation_ = &it->second;
    }

    currentGlobalTransforms_.resize(model_->staticSkeleton.GetBoneCount(), Mat4::identity());
    currentAnimationTime_ = 0;
}

DiffuseSolidMesh::DiffuseSolidMesh(
    const Mat4 &worldMatrix, const Vec4 &brightness,
    std::shared_ptr<const DiffuseSolidMeshEffect> effect, std::shared_ptr<Model> model,
    const Mesh::SkeletonAnimation *currentAnimation, bool animationLoop,
    std::vector<Mat4> currentGlobalTransforms, float currentAnimationTime)
{
    worldMatrix_ = worldMatrix;
    brightness_  = brightness;

    effect_ = std::move(effect);
    model_  = std::move(model);

    currentAnimation_ = currentAnimation;
    animationLoop_    = animationLoop;

    currentGlobalTransforms_ = std::move(currentGlobalTransforms);
    currentAnimationTime_    = currentAnimationTime;
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
