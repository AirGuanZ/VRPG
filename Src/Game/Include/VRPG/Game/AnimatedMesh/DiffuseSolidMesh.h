#pragma once

#include <VRPG/Game/AnimatedMesh/BasicEffect/DiffuseSolidMeshEffect.h>
#include <VRPG/Game/AnimatedMesh/SubMesh.h>

VRPG_GAME_BEGIN

class AnimatedDiffuseSolidMesh : public agz::misc::uncopyable_t
{
public:

    AnimatedDiffuseSolidMesh(
        std::shared_ptr<const DiffuseSolidMeshEffect> effect,
        const Mesh::Mesh &mesh, std::map<std::string, ComPtr<ID3D11ShaderResourceView>> diffuseTextures,
        std::string_view initAnimationName = "");

    const Mesh::SkeletonAnimation *GetCurrentAnimation() const noexcept;

    void SetCurrentAnimation(std::string_view &animationName);

    bool IsAnimationLoopEnabled() const noexcept;

    void EnableAnimationLoop(bool loop);

    bool IsAnimationEnd() const noexcept;

    float GetCurrentAnimationTime() const noexcept;

    void SetCurrentAnimationTime(float timePoint);

    void RenderForward(const ForwardRenderParams &params) const;

    void RenderShadow(const ShadowRenderParams &params) const;

    void SetWorldTransform(const Mat4 &worldMatrix);

    void UpdateBoneTransform();

private:

    using Vertex  = DiffuseSolidMeshEffect::Vertex;
    using Index   = uint32_t;
    using Submesh = AnimatedSubMesh<Vertex, Index>;

    static void InitializeSubmesh(const Mesh::MeshComponent &component, Submesh &submesh);

    // 位置

    Mat4 worldMatrix_;

    // 亮度

    Vec4 brightness_;

    // 管线数据

    std::shared_ptr<const DiffuseSolidMeshEffect> effect_;

    // 模型数据

    struct MeshComponent
    {
        std::unique_ptr<Submesh> submesh;
        Mat4 bindingTransform_;
        ShaderResourceView diffuseTexture_;
        int boneIndex = -1;
    };

    Mesh::StaticSkeleton                                        staticSkeleton_;
    std::vector<MeshComponent>                                  meshComponents_;
    std::map<std::string, Mesh::SkeletonAnimation, std::less<>> skeletonAnimations_;

    // 动画设置

    const Mesh::SkeletonAnimation *currentAnimation_;
    bool animationLoop_;

    // 动画状态

    std::vector<Mat4> currentGlobalTransforms_;
    float currentAnimationTime_;
};

VRPG_GAME_END
