#pragma once

#include <libconfig.h++>

#include <VRPG/Game/Mesh/Animation/AnimationModel.h>
#include <VRPG/Game/Mesh/Animation/AnimationState.h>
#include <VRPG/Game/Mesh/MeshComponent.h>
#include <VRPG/Game/Mesh/BasicEffect/DiffuseSolidMeshEffect.h>
#include <VRPG/Mesh/Mesh.h>

VRPG_GAME_BEGIN

class DiffuseSolidMesh : public agz::misc::uncopyable_t
{
public:

    struct LoadParams
    {
        std::shared_ptr<const DiffuseSolidMeshEffect> effect;
        std::string                                   meshFilename;
        std::map<std::string, std::string>            diffuseTextureFilenames;
    };

    static std::unique_ptr<DiffuseSolidMesh> LoadFromConfig(
        std::shared_ptr<const DiffuseSolidMeshEffect> effect, const libconfig::Setting &config);

    static std::unique_ptr<DiffuseSolidMesh> LoadFromFile(const LoadParams &params);

    std::unique_ptr<DiffuseSolidMesh> Clone() const;

    const std::shared_ptr<const DiffuseSolidMeshEffect> &GetEffect() const noexcept;

    const Mesh::SkeletonAnimation *GetCurrentAnimation() const noexcept;

    void SetCurrentAnimation(std::string_view animationName);

    bool IsAnimationLoopEnabled() const noexcept;

    void EnableAnimationLoop(bool loop);

    bool IsAnimationEnd() const noexcept;

    float GetCurrentAnimationTime() const noexcept;

    void SetCurrentAnimationTime(float timePoint);

    void RenderForward(const ForwardRenderParams &params) const;

    void RenderShadow(const ShadowRenderParams &params) const;

    void SetWorldTransform(const Mat4 &worldMatrix);

    void SetBrightness(const Vec4 &brightness);

    void UpdateBoneTransform();

private:

    using Vertex       = DiffuseSolidMeshEffect::Vertex;
    using Index        = uint32_t;
    using Submesh      = MeshComponent<Vertex, Index>;
    using AnimationMap = std::map<std::string, Mesh::SkeletonAnimation, std::less<>>;

    struct MeshBinding
    {
        std::unique_ptr<Submesh> submesh;
        Mat4                     nonbindingTransform;
        Mat4                     bindingTransform;
        ShaderResourceView       diffuseTexture;
        int                      boneIndex = -1;
    };

    struct Model
    {
        std::shared_ptr<const DiffuseSolidMeshEffect> effect;
        std::vector<MeshBinding>                      meshComponents;
        std::shared_ptr<AnimationModel>               animationModel;
    };

    DiffuseSolidMesh(
        const Mesh::Mesh &mesh, std::shared_ptr<const DiffuseSolidMeshEffect> effect,
        std::map<std::string, ComPtr<ID3D11ShaderResourceView>> diffuseTextures);

    DiffuseSolidMesh(
        const Mat4 &worldMatrix, const Vec4 &brightness, std::shared_ptr<Model> model,
        AnimationState animationState, std::vector<Mat4> currentGlobalTransforms);

    static void InitializeSubmesh(const Mesh::MeshComponent &component, Submesh &submesh);

    // 位置

    Mat4 worldMatrix_;

    // 亮度

    Vec4 brightness_;

    // 模型数据

    std::shared_ptr<Model> model_;

    // 动画设置

    AnimationModel animationModel_;

    // 动画状态

    std::vector<Mat4> currentGlobalTransforms_;
    AnimationState    animationState_;
};

VRPG_GAME_END
