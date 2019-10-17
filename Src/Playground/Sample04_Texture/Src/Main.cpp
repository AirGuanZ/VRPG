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
        Vec2 pos;
        Vec2 uv;
    };

    VertexBuffer<Vertex> vertexBuffer_;
    InputLayout inputLayout_;

    Shader<SS_VS, SS_PS> shader_;
    UniformManager<SS_VS, SS_PS> uniforms_;

    ShaderResourceView textureSRV_;
    Sampler sampler_;

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

protected:

    WindowDesc GetWindowDesc() override
    {
        WindowDesc desc;
        desc.windowTitle = L"Sample04 Texture";
        return desc;
    }

    void Frame() override
    {
        window_.DoEvents();

        if(keyboard_.IsKeyPressed(KEY_ESCAPE))
            window_.SetCloseFlag(true);

        static float backgroundColor[] = { 0.2f, 0.2f, 0.2f, 0 };
        window_.ClearDefaultDepthStencil();
        window_.ClearDefaultRenderTarget(backgroundColor);

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
