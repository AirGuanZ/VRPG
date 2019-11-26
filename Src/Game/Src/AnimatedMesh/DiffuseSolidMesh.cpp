#include <VRPG/Game/AnimatedMesh/DiffuseSolidMesh.h>

VRPG_GAME_BEGIN

AnimatedDiffuseSolidMesh::AnimatedDiffuseSolidMesh(
    std::shared_ptr<const DiffuseSolidMeshEffect> effect,
    const Mesh::Mesh &mesh, std::map<std::string, ComPtr<ID3D11ShaderResourceView>> diffuseTextures,
    std::string_view initAnimationName)
{
    worldMatrix_ = Mat4::identity();

    effect_ = std::move(effect);

    staticSkeleton_     = mesh.staticSkeleton;
    skeletonAnimations_ = mesh.skeletonAnimations;

    for(auto &m : mesh.staticComponents)
    {
        MeshComponent component;

        component.submesh = std::make_unique<Submesh>();
        InitializeSubmesh(m.mesh, *component.submesh);

        component.bindingTransform_ = m.bindingTransform;

        auto it = diffuseTextures.find(m.name);
        if(it == diffuseTextures.end())
        {
            throw VRPGGameException("texture not found for diffuse solid submesh: " + m.name);
        }
        component.diffuseTexture_ = it->second;

        component.boneIndex = m.boneIndex;
    }

    animationLoop_ = false;
    if(initAnimationName.empty())
    {
        currentAnimation_ = nullptr;
    }
    else
    {
        auto it = skeletonAnimations_.find(initAnimationName);
        if(it == skeletonAnimations_.end())
        {
            throw VRPGGameException("animation not found: " + std::string(initAnimationName));
        }
        currentAnimation_ = &it->second;
    }

    currentGlobalTransforms_.resize(staticSkeleton_.GetBoneCount(), Mat4::identity());
    currentAnimationTime_ = 0;
}

const Mesh::SkeletonAnimation *AnimatedDiffuseSolidMesh::GetCurrentAnimation() const noexcept
{
    return currentAnimation_;
}

void AnimatedDiffuseSolidMesh::SetCurrentAnimation(std::string_view &animationName)
{
    if(animationName.empty())
    {
        currentAnimation_ = nullptr;
        currentAnimationTime_ = 0;
        return;
    }

    auto it = skeletonAnimations_.find(animationName);
    if(it == skeletonAnimations_.end())
    {
        throw VRPGGameException("animation not found: " + std::string(animationName));
    }
    currentAnimation_ = &it->second;
    currentAnimationTime_ = 0;
}

bool AnimatedDiffuseSolidMesh::IsAnimationLoopEnabled() const noexcept
{
    return animationLoop_;
}

void AnimatedDiffuseSolidMesh::EnableAnimationLoop(bool loop)
{
    animationLoop_ = loop;
}

bool AnimatedDiffuseSolidMesh::IsAnimationEnd() const noexcept
{
    if(!currentAnimation_)
    {
        return false;
    }
    return currentAnimationTime_ > currentAnimation_->GetEndTime();
}

float AnimatedDiffuseSolidMesh::GetCurrentAnimationTime() const noexcept
{
    return currentAnimationTime_;
}

void AnimatedDiffuseSolidMesh::SetCurrentAnimationTime(float timePoint)
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

void AnimatedDiffuseSolidMesh::RenderForward(const ForwardRenderParams &params) const
{
    const Mat4 &viewProj = params.camera->GetViewProjectionMatrix();

    for(auto &mesh : meshComponents_)
    {
        Mat4 world;
        if(int boneIndex = mesh.boneIndex; boneIndex >= 0)
        {
            world = mesh.bindingTransform_ * currentGlobalTransforms_[boneIndex] * worldMatrix_;
        }
        else
        {
            world = mesh.bindingTransform_ * worldMatrix_;
        }
        Mat4 worldViewProj = world * viewProj;

        effect_->BindForwardVSTransform(world, worldViewProj);
        effect_->BindForwardPSBrightness(brightness_);
        effect_->BindForwardPSDiffuseTexture(mesh.diffuseTexture_);

        mesh.submesh->Render();
    }
}

void AnimatedDiffuseSolidMesh::RenderShadow(const ShadowRenderParams &params) const
{
    for(auto &mesh : meshComponents_)
    {
        Mat4 world;
        if(int boneIndex = mesh.boneIndex; boneIndex >= 0)
        {
            world = mesh.bindingTransform_ * currentGlobalTransforms_[boneIndex] * worldMatrix_;
        }
        else
        {
            world = mesh.bindingTransform_ * worldMatrix_;
        }
        Mat4 worldViewProj = world * params.shadowViewProj;

        effect_->BindShadowVSTransform(worldViewProj);

        mesh.submesh->Render();
    }
}

void AnimatedDiffuseSolidMesh::SetWorldTransform(const Mat4 &worldMatrix)
{
    worldMatrix_ = worldMatrix;
}

void AnimatedDiffuseSolidMesh::UpdateBoneTransform()
{
    if(!currentAnimation_)
    {
        staticSkeleton_.ComputeStaticTransformMatrix(currentGlobalTransforms_.data());
        return;
    }
    currentAnimation_->ComputeTransformMatrix(staticSkeleton_, currentAnimationTime_, currentGlobalTransforms_.data());
}

void AnimatedDiffuseSolidMesh::InitializeSubmesh(const Mesh::MeshComponent &component, Submesh &submesh)
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
