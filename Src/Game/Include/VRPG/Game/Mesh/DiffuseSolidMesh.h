#pragma once

#include <libconfig.h++>

#include <VRPG/Game/Mesh/Animation/AnimationModel.h>
#include <VRPG/Game/Mesh/Animation/AnimationState.h>
#include <VRPG/Game/Mesh/MeshComponent.h>
#include <VRPG/Game/Mesh/BasicEffect/DiffuseSolidMeshEffect.h>
#include <VRPG/Mesh/Mesh.h>

VRPG_GAME_BEGIN

/**
 * @brief 使用diffuse shading model的网格模型，支持骨骼动画
 */
class DiffuseSolidMesh : public agz::misc::uncopyable_t
{
public:

    struct LoadParams
    {
        std::shared_ptr<const DiffuseSolidMeshEffect> effect;
        std::string                                   meshFilename;
        std::map<std::string, std::string>            diffuseTextureFilenames;
    };

    /**
     * @brief 从配置选项中创建一个diffuse solid mesh实例
     * {
     *     Filename = "mesh file name";
     *     DiffuseTexture = {
     *         Name0 = "texture filename";
     *         Name1 = "texture filename";
     *         //...
     *     };
     * }
     */
    static std::unique_ptr<DiffuseSolidMesh> LoadFromConfig(
        std::shared_ptr<const DiffuseSolidMeshEffect> effect, const libconfig::Setting &config);

    /**
     * @brief 从mesh文件和纹理文件中创建一个diffuse solid mesh实例
     */
    static std::unique_ptr<DiffuseSolidMesh> LoadFromFile(const LoadParams &params);

    /**
     * @brief clone一个完全一样的mesh实例
     */
    std::unique_ptr<DiffuseSolidMesh> Clone() const;

    /**
     * @brief 取得rendering effect
     */
    const std::shared_ptr<const DiffuseSolidMeshEffect> &GetEffect() const noexcept;

    /**
     * @brief 取得当前正在播放的动画
     *
     * 无正播放的动画时返回nullptr
     */
    const Mesh::SkeletonAnimation *GetCurrentAnimation() const noexcept;

    /**
     * @brief 设置当前播放的动画名
     *
     * 播放时间会自动清零
     *
     * 若不存在具有该名字的动画，会切换到静态骨骼
     */
    void SetCurrentAnimation(std::string_view animationName);

    /**
     * @brief 是否开启了循环播放
     */
    bool IsAnimationLoopEnabled() const noexcept;

    /**
     * @brief 开启或关闭循环播放
     */
    void EnableAnimationLoop(bool loop);

    /**
     * @brief 当前动画是否已经播放完毕
     */
    bool IsAnimationEnd() const noexcept;

    /**
     * @brief 取得当前播放动画的时间点
     */
    float GetCurrentAnimationTime() const noexcept;

    /**
     * @brief 设置当前播放动画的时间点
     *
     * 若开启了循环播放，超出当前动画结束时间的值会对结束时间取余
     */
    void SetCurrentAnimationTime(float timePoint);

    /**
     * @brief 执行前向渲染
     *
     * 要求对应effect的StartForward已被调用
     */
    void RenderForward(const ForwardRenderParams &params) const;

    /**
     * @brief 执行阴影渲染
     *
     * 要求对应effect的StartShadow已被调用
     */
    void RenderShadow(const ShadowRenderParams &params) const;

    /**
     * @brief 设置local to world transform matrix
     */
    void SetWorldTransform(const Mat4 &worldMatrix);

    /**
     * @brief 设置整体亮度
     */
    void SetBrightness(const Vec4 &brightness);

    /**
     * @brief 计算骨骼变换矩阵，应在每帧渲染前调用
     */
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

    struct DiffuseSolidMeshModel
    {
        std::shared_ptr<const DiffuseSolidMeshEffect> effect;
        std::vector<MeshBinding>                      meshComponents;
        std::shared_ptr<AnimationModel>               animationModel;
    };

    DiffuseSolidMesh(
        const Mesh::Mesh &mesh, std::shared_ptr<const DiffuseSolidMeshEffect> effect,
        std::map<std::string, ComPtr<ID3D11ShaderResourceView>> diffuseTextures);

    DiffuseSolidMesh(
        const Mat4 &worldMatrix, const Vec4 &brightness, std::shared_ptr<DiffuseSolidMeshModel> model,
        AnimationState animationState, std::vector<Mat4> currentGlobalTransforms);

    static void InitializeSubmesh(const Mesh::MeshComponent &component, Submesh &submesh);

    // 位置

    Mat4 worldMatrix_;

    // 亮度

    Vec4 brightness_;

    // 模型数据

    std::shared_ptr<DiffuseSolidMeshModel> model_;

    // 动画状态

    std::vector<Mat4> currentGlobalTransforms_;
    AnimationState    animationState_;
};

VRPG_GAME_END
