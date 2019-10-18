#include <iostream>

#include <agz/utility/file.h>
#include <agz/utility/misc.h>

#include <SampleApp.h>

using namespace VRPG::Base;
using namespace D3D;

class Sample04_Texture : public SampleApp
{
    struct Vertex
    {
        Vec3 pos;
        Vec3 nor;
        Vec2 uv;
    };

    struct TransformConstantBuffer
    {
        Mat4 WVP;
    };

    VertexBuffer<Vertex> vertexBuffer_;
    IndexBuffer<uint16_t> indexBuffer_;
    InputLayout inputLayout_;

    Shader<SS_VS, SS_PS> shader_;
    UniformManager<SS_VS, SS_PS> uniforms_;

    float rotateRadian_ = 0;
    ConstantBuffer<TransformConstantBuffer> transformConstantBuffer_;

    ShaderResourceView textureSRV_;
    Sampler sampler_;

    KeyDownHandler keyDownHandler_;

    void InitializeShader()
    {
        std::string vertexSource = agz::file::read_txt_file("Asset/Playground/Sample04_Texture/vertex.hlsl");
        std::string pixelSource  = agz::file::read_txt_file("Asset/Playground/Sample04_Texture/pixel.hlsl");
        shader_.InitializeStage<SS_VS>(vertexSource);
        shader_.InitializeStage<SS_PS>(pixelSource);
        if(!shader_.IsAllStagesAvailable())
            throw std::runtime_error("failed to initialize shader");
        uniforms_ = shader_.CreateUniformManager();
    }

    void InitializeConstantBuffer()
    {
        transformConstantBuffer_.Initialize(true, nullptr);
        uniforms_.GetConstantBufferSlot<SS_VS>("Transform")->SetBuffer(transformConstantBuffer_);
    }

    void InitializeVertexBuffer()
    {
        Vertex vertexData[] =
        {
            // +x
            { { 1, -1, -1 }, { 1, 0, 0 }, { 0, 1 } },
            { { 1, 1, -1 }, { 1, 0, 0 }, { 0, 0 } },
            { { 1, 1, 1 }, { 1, 0, 0 }, { 1, 0 } },
            { { 1, -1, 1 }, { 1, 0, 0 }, { 1, 1 } },

            // -x
            { { -1, -1, 1 }, { -1, 0, 0 }, { 0, 1 } },
            { { -1, 1, 1 }, { -1, 0, 0 }, { 0, 0 } },
            { { -1, 1, -1 }, { -1, 0, 0 }, { 1, 0 } },
            { { -1, -1, -1 }, { -1, 0, 0 }, { 1, 1 } },

            // +y
            { { -1, 1, -1 }, { 0, 1, 0 }, { 0, 1 } },
            { { -1, 1, 1 }, { 0, 1, 0 }, { 0, 0 } },
            { { 1, 1, 1 }, { 0, 1, 0 }, { 1, 0 } },
            { { 1, 1, -1 }, { 0, 1, 0 }, { 1, 1 } },

            // -y
            { {  1, -1, 1}, { 0, -1, 0 }, { 0, 1 } },
            { { -1, -1, 1 }, { 0, -1, 0 }, { 1, 1 } },
            { { -1, -1, -1 }, { 0, -1, 0 }, { 1,  0 } },
            { { 1, -1, -1 }, { 0, -1, 0 }, { 0, 0 } },

            // +z
            { { 1, -1, 1 }, { 0, 0, 1 }, { 0, 1 } },
            { { 1, 1, 1 }, { 0, 0, 1 }, { 0, 0 } },
            { { -1, 1, 1 }, { 0, 0, 1 }, { 1, 0 } },
            { { -1, -1, 1 }, { 0, 0, 1 }, { 1, 1 } },

            // -z
            { { -1, -1, -1 }, { 0, 0, -1 }, { 0, 1 } },
            { { -1, 1, -1 }, { 0, 0, -1 }, { 0, 0 } },
            { { 1, 1, -1 }, { 0, 0, -1 }, { 1, 0 } },
            { { 1, -1, -1 }, { 0, 0, -1 }, { 1, 1 } }
        };
        vertexBuffer_.Initialize(24, false, vertexData);

        uint16_t indexData[] =
        {
            0,  1,  2,  0,  2,  3,
            4,  5,  6,  4,  6,  7,
            8,  9,  10, 8,  10, 11,
            12, 13, 14, 12, 14, 15,
            16, 17, 18, 16, 18, 19,
            20, 21, 22, 20, 22, 23
        };
        indexBuffer_.Initialize(36, false, indexData);
    }

    void InitializeInputLayout()
    {
        inputLayout_ = InputLayoutBuilder()
            ("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0)
            ("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, UINT(agz::byte_offset(&Vertex::nor)))
            ("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, UINT(agz::byte_offset(&Vertex::uv)))
            .Build(shader_.GetVertexShaderByteCode());
    }

    void InitializeTexture()
    {
        textureSRV_ = Texture2DLoader::LoadShaderResourceViewFromImage("Asset/Playground/Sample04_Texture/texture.png");
        uniforms_.GetShaderResourceSlot<SS_PS>("color")->SetShaderResourceView(textureSRV_);
    }

    void InitializeSampler()
    {
        sampler_.Initialize(D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR);
        uniforms_.GetSamplerSlot<SS_PS>("colorSampler")->SetSampler(sampler_);
    }

protected:

    WindowDesc GetWindowDesc() override
    {
        WindowDesc desc;
        desc.windowTitle = L"Sample04 Texture";
        return desc;
    }

    void Initialize() override
    {
        InitializeShader();
        InitializeConstantBuffer();
        InitializeVertexBuffer();
        InitializeInputLayout();
        InitializeTexture();
        InitializeSampler();

        rotateRadian_ = 0;

        keyDownHandler_.SetFunction([&](const KeyDownEvent &e)
        {
            if(e.key == KEY_ESCAPE)
                window_.SetCloseFlag(true);
        });
        keyboard_->Attach(&keyDownHandler_);

        shader_.Bind();
        uniforms_.Bind();
        inputLayout_.Bind();
        vertexBuffer_.Bind(0);
        indexBuffer_.Bind();
    }

    void Frame() override
    {
        window_.DoEvents();

        static float backgroundColor[] = { 0.2f, 0.2f, 0.2f, 0 };
        window_.ClearDefaultDepthStencil();
        window_.ClearDefaultRenderTarget(backgroundColor);

        Mat4 WVP =
            Trans4::rotate_x(0.4f)
            * Trans4::rotate_z(0.8f)
            * Trans4::rotate_y(rotateRadian_)
            * Trans4::look_at({ 0, 0, -5 }, { 0, 0, 0 }, { 0, 1, 0 })
            * Trans4::perspective(agz::math::deg2rad(60.0f), window_.GetClientAspectRatio(), 0.01f, 100.0f);
        transformConstantBuffer_.SetValue({ WVP });
        rotateRadian_ += 0.015f;

        RenderState::DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, indexBuffer_.GetIndexCount());

        window_.SwapBuffers();
    }
};

int main()
{
    try
    {
        Sample04_Texture app;
        app.Run();
    }
    catch(const std::exception &err)
    {
        std::cout << err.what() << std::endl;
        return 1;
    }
}
