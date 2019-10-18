#include <iostream>

#include <agz/utility/file.h>

#include <SampleApp.h>

using namespace VRPG::Base;
using namespace D3D;

class Sample02_Triangle : public SampleApp
{
    struct Vertex
    {
        Vec2 position;
        Vec3 color;
    };

    struct TransformConstantBuffer
    {
        Mat4 transform;
    };

    VertexBuffer<Vertex> vertexBuffer_;
    InputLayout inputLayout_;

    Shader<SS_VS, SS_PS> shader_;
    UniformManager<SS_VS, SS_PS> uniforms_;

    float rotateRadian_ = 0.0f;
    ConstantBuffer<TransformConstantBuffer> transformBuffer_;

    RasterizerState rasterizerState_;

    KeyDownHandler keyDownHandler_;

    void InitializeShader()
    {
        std::string vertexSource = agz::file::read_txt_file("Asset/Playground/Sample02_Triangle/vertex.hlsl");
        std::string pixelSource  = agz::file::read_txt_file("Asset/Playground/Sample02_Triangle/pixel.hlsl");
        shader_.InitializeStage<SS_VS>(vertexSource);
        shader_.InitializeStage<SS_PS>(pixelSource);
        if(!shader_.IsAllStagesAvailable())
            throw std::runtime_error("failed to initialize shader");
        uniforms_ = shader_.CreateUniformManager();
    }
    
    void InitializeConstantBuffer()
    {
        transformBuffer_.Initialize(true, nullptr);
        uniforms_.GetConstantBufferSlot<SS_VS>("Transform")->SetBuffer(transformBuffer_);
    }

    void InitializeInputLayout()
    {
        inputLayout_ = InputLayoutBuilder()
            ("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0)
            ("COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, UINT(agz::byte_offset(&Vertex::color)))
            .Build(shader_.GetVertexShaderByteCode());
    }

    void InitializeVertexBuffer()
    {
        Vertex vertexData[] =
        {
            { { -1, -1 }, { 1, 0, 0 } },
            { { 0, 1 }, { 0, 1, 0 } },
            { { 1, -1 }, { 0, 0, 1 } }
        };

        vertexBuffer_.Initialize(3, false, vertexData);
    }

    void InitializeRasterizerState()
    {
        rasterizerState_.Initialize(D3D11_FILL_SOLID, D3D11_CULL_NONE, false);
    }

protected:

    WindowDesc GetWindowDesc() override
    {
        WindowDesc desc;
        desc.windowTitle = L"Sample02 Triangle";
        return desc;
    }

    void Initialize() override
    {
        keyDownHandler_.SetFunction([&](const KeyDownEvent &e)
        {
            if(e.key == KEY_ESCAPE)
                window_.SetCloseFlag(true);
        });
        keyboard_->Attach(&keyDownHandler_);

        InitializeShader();
        InitializeConstantBuffer();
        InitializeInputLayout();
        InitializeVertexBuffer();
        InitializeRasterizerState();

        rotateRadian_ = 0.0f;
    }

    void Frame() override
    {
        window_.DoEvents();

        window_.ClearDefaultDepthStencil();
        window_.ClearDefaultRenderTarget();

        Mat4 WVP =
            Trans4::rotate_y(rotateRadian_)
          * Trans4::look_at({ 0, 0, -3 }, { 0, 0, 0 }, { 0, 1, 0 })
          * Trans4::perspective(agz::math::deg2rad(60.0f), window_.GetClientAspectRatio(), 0.1f, 100.0f);
        transformBuffer_.SetValue({ WVP });
        rotateRadian_ += 0.01f;

        shader_.Bind();
        uniforms_.Bind();
        rasterizerState_.Bind();
        inputLayout_.Bind();
        vertexBuffer_.Bind(0);

        RenderState::Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, vertexBuffer_.GetVertexCount());

        vertexBuffer_.Unbind(0);
        inputLayout_.Unbind();
        rasterizerState_.Unbind();
        uniforms_.Unbind();
        shader_.Unbind();

        window_.SwapBuffers();
    }
};

int main()
{
    try
    {
        Sample02_Triangle app;
        app.Run();
    }
    catch(const std::exception &err)
    {
        std::cout << err.what() << std::endl;
        return 1;
    }
}
