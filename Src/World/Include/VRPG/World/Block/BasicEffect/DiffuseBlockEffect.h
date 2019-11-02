#pragma once

#include <optional>

#include <agz/utility/texture.h>

#include <VRPG/World/Block/BlockEffect.h>

VRPG_WORLD_BEGIN

class DiffuseBlockEffect;

/**
 * 一个DiffuseBlockEffect中存有一个表示Diffuse Albedo的Texture Array，
 * 而单个Texture Array未必能存下所有需要的Texture，所以可能会有多个DiffuseBlockEffect实例。
 * 这些实例共享Shader、Constant Buffer等，只有Texture Array不同。
 *
 * Texture Array的分配就由DiffuseBlockEffect完成
 */
class DiffuseBlockEffectGenerator
{
public:

    struct Vertex
    {
        Vec3 position;
        Vec2 texCoord;
        uint32_t texIndex;
        Vec4 brightness;
    };

    struct VS_Transform
    {
        Mat4 WVP;
    };

    struct PS_Sky
    {
        Vec3 skyLight;
        float pad = 0;
    };

    /**
     * @brief 由多个DiffuseBlockEffect实例所共享的数据
     */
    struct CommonProperties
    {
        CommonProperties();

        Shader<SS_VS, SS_PS> shader_;
        UniformManager<SS_VS, SS_PS> uniforms_;

        InputLayout inputLayout_;

        ConstantBuffer<VS_Transform> vsTransform_;
        ConstantBuffer<PS_Sky> psSky_;

        ShaderResourceSlot<SS_PS> *diffuseTextureSlot_;
    };

    DiffuseBlockEffectGenerator(int textureSize, int expectedArraySize);

    /**
     * @brief array是否已经被填满
     *
     * 若返回true，则继续调用AddTexture之前必须先通过InitializeEffect释放出去
     */
    bool IsFull() const noexcept;

    /**
     * @brief array是否为空
     */
    bool IsEmpty() const noexcept;

    /**
     * @brief array中是否还能再增加arrayDataCount个texture data
     */
    bool HasEnoughSpaceFor(int arrayDataCount) const noexcept;

    /**
     * @brief 向texture array中添加一个textureSize^2大小的texture data
     *
     * 返回新添加的texture data在texture array中的下标
     */
    int AddTexture(const Vec3 *data);
	
	/**
	 * @brief 初始化给定的effect
 	 */
	void InitializeEffect(DiffuseBlockEffect &effect);

private:

    std::shared_ptr<CommonProperties> commonProperties_;

    std::vector<agz::texture::texture2d_t<Vec3>> textureArrayData_;
    int textureSize_;
    int maxArraySize_;

    int generatedEffectCount_;
};

class DiffuseBlockEffect : public BlockEffect
{
public:

    using Generator = DiffuseBlockEffectGenerator;
    using Vertex = Generator::Vertex;

    class Builder : public PartialSectionModelBuilder
    {
        std::vector<Vertex> vertices_;
        std::vector<VertexIndex> indices_;
        const DiffuseBlockEffect *effect_;

    public:

        explicit Builder(const DiffuseBlockEffect *effect) noexcept;

        void AddVertex(const Vertex &vertex);

        void AddIndexedTriangle(uint16_t indexA, uint16_t indexB, uint16_t indexC);

        size_t GetVertexCount() const noexcept { return vertices_.size(); }

        std::shared_ptr<const PartialSectionModel> Build() const override;
    };

	void Initialize(
		std::shared_ptr<Generator::CommonProperties> commonProperties,
		ShaderResourceView textureArray, int semanticsIndex);

    const char *GetName() const override;

    void Bind() const override;

    void Unbind() const override;

    std::unique_ptr<PartialSectionModelBuilder> CreateModelBuilder() const override;

    void SetRenderParams(const BlockRenderParams &params) const override;

private:

    std::shared_ptr<Generator::CommonProperties> commonProperties_;

    ShaderResourceView textureArray_;
    std::string name_;
};

VRPG_WORLD_END
