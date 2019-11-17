#pragma once

#include <VRPG/Game/Camera/Camera.h>

VRPG_GAME_BEGIN

class ChosenWireframeRenderer : public agz::misc::uncopyable_t
{
public:

    ChosenWireframeRenderer();

    void DrawBlockWireframeAt(const Camera &camera, const Vec3i &globalBlockPosition);

private:

    struct Vertex
    {
        Vec3 position;
    };

    struct VSTransform
    {
        Mat4 WVP;
    };

    VertexBuffer<Vertex>         vertexBuffer_;
    IndexBuffer<uint16_t>        indexBuffer_;

    Shader<SS_VS, SS_PS>         shader_;
    InputLayout                  inputLayout_;

    UniformManager<SS_VS, SS_PS> uniforms_;
    ConstantBuffer<VSTransform>  vsTransform_;

    RasterizerState rasterizerState_;
    DepthState      depthState_;
};

VRPG_GAME_END
