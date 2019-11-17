#include <VRPG/Game/Misc/ChosenWireframe.h>

VRPG_GAME_BEGIN

namespace
{
    const char *VERTEX_SHADER_SOURCE = R"___(
cbuffer Transform
{
    float4x4 WVP;
};

struct VSInput
{
    float3 position : POSITION;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.position = mul(float4(input.position, 1), WVP);
    return output;
}
)___";

    const char *PIXEL_SHADER_SOURCE = R"___(
struct PSInput
{
    float4 position : SV_POSITION;
};

float4 main(PSInput input) : SV_TARGET
{
    return float4(0, 0, 0, 1);
}
)___";
}

ChosenWireframeRenderer::ChosenWireframeRenderer()
{
    constexpr float OUTER_OFFSET = 0.02f;

    const Vertex vertexData[] =
    {
        { { 0 - OUTER_OFFSET, 0 - OUTER_OFFSET, 0 - OUTER_OFFSET } },
        { { 0 - OUTER_OFFSET, 1 + OUTER_OFFSET, 0 - OUTER_OFFSET } },
        { { 1 + OUTER_OFFSET, 1 + OUTER_OFFSET, 0 - OUTER_OFFSET } },
        { { 1 + OUTER_OFFSET, 0 - OUTER_OFFSET, 0 - OUTER_OFFSET } },
        { { 0 - OUTER_OFFSET, 0 - OUTER_OFFSET, 1 + OUTER_OFFSET } },
        { { 0 - OUTER_OFFSET, 1 + OUTER_OFFSET, 1 + OUTER_OFFSET } },
        { { 1 + OUTER_OFFSET, 1 + OUTER_OFFSET, 1 + OUTER_OFFSET } },
        { { 1 + OUTER_OFFSET, 0 - OUTER_OFFSET, 1 + OUTER_OFFSET } }
    };
    vertexBuffer_.Initialize(UINT(agz::array_size(vertexData)), false, vertexData);

    const uint16_t indexData[] =
    {
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        1, 5, 2, 6, 0, 4, 3, 7
    };
    indexBuffer_.Initialize(UINT(agz::array_size(indexData)), false, indexData);

    shader_.InitializeStage<SS_VS>(VERTEX_SHADER_SOURCE);
    shader_.InitializeStage<SS_PS>(PIXEL_SHADER_SOURCE);
    if(!shader_.IsAllStagesAvailable())
        throw VRPGGameException("failed to initialize shader in ChosenWireframeRenderer");

    inputLayout_ = InputLayoutBuilder
        ("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(Vertex, position))
        .Build(shader_.GetVertexShaderByteCode());

    uniforms_ = shader_.CreateUniformManager();
    vsTransform_.Initialize(true, nullptr);
    uniforms_.GetConstantBufferSlot<SS_VS>("Transform")->SetBuffer(vsTransform_);

    rasterizerState_.Initialize(D3D11_FILL_WIREFRAME, D3D11_CULL_NONE, false);

    depthState_.Initialize(true, D3D11_DEPTH_WRITE_MASK_ZERO);
}

void ChosenWireframeRenderer::DrawBlockWireframeAt(const Camera &camera, const Vec3i &block)
{
    Mat4 WVP = Trans4::translate(block.map([](int i) { return float(i); }))
             * camera.GetViewProjectionMatrix();
    vsTransform_.SetValue({ WVP });

    shader_         .Bind();
    uniforms_       .Bind();
    inputLayout_    .Bind();
    vertexBuffer_   .Bind(0);
    indexBuffer_    .Bind();
    rasterizerState_.Bind();
    depthState_     .Bind();

    RenderState::DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY_LINELIST, indexBuffer_.GetIndexCount());
    
    shader_         .Unbind();
    uniforms_       .Unbind();
    inputLayout_    .Unbind();
    vertexBuffer_   .Unbind(0);
    indexBuffer_    .Unbind();
    rasterizerState_.Unbind();
    depthState_     .Unbind();
}

VRPG_GAME_END
