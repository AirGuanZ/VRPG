#include <iostream>

#include <agz/utility/file.h>
#include <agz/utility/misc.h>
#include <agz/utility/time.h>

#include <SampleApp.h>

using namespace VRPG::Base;
using namespace D3D;

class Camera
{
    Vec3 position_;

    float vertRadian_ = 0;
    float horiRadian_ = 0;

    static constexpr int CursorHistorySize = 6;

    float vertDeltaHistory[CursorHistorySize] = { 0 };
    float horiDeltaHistory[CursorHistorySize] = { 0 };

    float FOVYDegree_ = 60;
    float moveSpeed_ = 0.02f;
    float viewSpeed_ = 0.0015f;

public:

    struct CameraUpdateInput
    {
        bool leftKey  = false;
        bool rightKey = false;
        bool frontKey = false;
        bool backKey  = false;
        bool upKey    = false;
        bool downKey  = false;

        float relativeCursorX_ = 0;
        float relativeCursorY_ = 0;
    };

    void SetPosition(const Vec3 &position) noexcept
    {
        position_ = position;
    }

    void SetFOVy(float degree) noexcept
    {
        FOVYDegree_ = agz::math::clamp<float>(degree, 20, 120);
    }

    void SetMoveSpeed(float speed) noexcept
    {
        moveSpeed_ = speed;
    }

    void SetViewSpeed(float speed) noexcept
    {
        viewSpeed_ = speed;
    }

    void Update(const CameraUpdateInput &input) noexcept
    {
        for(int i = CursorHistorySize - 2; i >= 0; --i)
        {
            vertDeltaHistory[i + 1] = vertDeltaHistory[i];
            horiDeltaHistory[i + 1] = horiDeltaHistory[i];
        }
        vertDeltaHistory[0] = -input.relativeCursorY_;
        horiDeltaHistory[0] = -input.relativeCursorX_;

        float meanVertDelta = 0, meanHoriDelta = 0;
        for(int i = 0; i < CursorHistorySize; ++i)
        {
            meanVertDelta += vertDeltaHistory[i];
            meanHoriDelta += horiDeltaHistory[i];
        }

        horiRadian_ += viewSpeed_ * meanHoriDelta / CursorHistorySize;
        vertRadian_ += viewSpeed_ * meanVertDelta / CursorHistorySize;

        constexpr float lmt = agz::math::PI_f / 2 - 0.01f;
        vertRadian_ = agz::math::clamp(vertRadian_, -lmt, lmt);

        Vec3 moveDirection;

        Vec3 front = {
            std::cos(horiRadian_),
            0,
            std::sin(horiRadian_)
        };
        if(input.frontKey)
            moveDirection += front;
        if(input.backKey)
            moveDirection -= front;

        Vec3 left = { -front.z, 0, front.x };
        if(input.leftKey)
            moveDirection += left;
        if(input.rightKey)
            moveDirection -= left;

        if(input.upKey)
            moveDirection.y += 1;
        if(input.downKey)
            moveDirection.y -= 1;

        position_ += moveSpeed_ * moveDirection;
    }

    Mat4 ConstructViewProjectMatrix(float wOverH) const noexcept
    {
        Vec3 direction = {
            std::cos(vertRadian_) * std::cos(horiRadian_),
            std::sin(vertRadian_),
            std::cos(vertRadian_) * std::sin(horiRadian_)
        };
        Mat4 view = Trans4::look_at(position_, position_ + direction, { 0, 1, 0 });
        Mat4 proj = Trans4::perspective(agz::math::deg2rad(FOVYDegree_), wOverH, 0.01f, 100.0f);
        return view * proj;
    }
};

class Sample05_Camera : public SampleApp
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

    ConstantBuffer<TransformConstantBuffer> transformConstantBuffer_;

    ShaderResourceView textureSRV_;
    Sampler sampler_;

    KeyDownHandler keyDownHandler_;
    Camera camera_;

    agz::time::fps_counter_t fps_;

    void InitializeShader()
    {
        std::string vertexSource = agz::file::read_txt_file("Asset/Playground/Sample05_Camera/vertex.hlsl");
        std::string pixelSource  = agz::file::read_txt_file("Asset/Playground/Sample05_Camera/pixel.hlsl");
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
        textureSRV_ = Texture2DLoader::LoadShaderResourceViewFromImage("Asset/Playground/Sample05_Camera/texture.png");
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
        desc.windowTitle = L"Sample05 Camera";
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

        keyDownHandler_.SetFunction([&](const KeyDownEvent &e)
        {
            if(e.key == KEY_ESCAPE)
                window_.SetCloseFlag(true);
            else if(e.key == KEY_C)
            {
                mouse_->ShowCursor(!mouse_->IsCursorVisible());
                mouse_->SetCursorLock(!mouse_->IsCursorLocked(), window_.GetClientSizeX() / 2, window_.GetClientSizeY() / 2);
            }
        });
        keyboard_->Attach(&keyDownHandler_);

        shader_.Bind();
        uniforms_.Bind();
        inputLayout_.Bind();
        vertexBuffer_.Bind(0);
        indexBuffer_.Bind();
        
        mouse_->ShowCursor(false);
        mouse_->SetCursorLock(true, window_.GetClientSizeX() / 2, window_.GetClientSizeY() / 2);
        mouse_->UpdatePosition();

        camera_.SetPosition({ -5, 0, 0 });
        fps_.restart();
    }

    void Frame() override
    {
        window_.DoEvents();
        window_.ImGuiNewFrame();

        static float backgroundColor[] = { 0.2f, 0.2f, 0.2f, 0 };
        window_.ClearDefaultDepthStencil();
        window_.ClearDefaultRenderTarget(backgroundColor);

        Camera::CameraUpdateInput cameraInput;
        cameraInput.relativeCursorX_ = float(mouse_->GetRelativeCursorPositionX());
        cameraInput.relativeCursorY_ = float(mouse_->GetRelativeCursorPositionY());
        cameraInput.frontKey         = keyboard_->IsKeyPressed('W');
        cameraInput.backKey          = keyboard_->IsKeyPressed('S');
        cameraInput.leftKey          = keyboard_->IsKeyPressed('A');
        cameraInput.rightKey         = keyboard_->IsKeyPressed('D');
        cameraInput.upKey            = keyboard_->IsKeyPressed(KEY_SPACE);
        cameraInput.downKey          = keyboard_->IsKeyPressed(KEY_LSHIFT);
        camera_.Update(cameraInput);

        Mat4 WVP = camera_.ConstructViewProjectMatrix(window_.GetClientAspectRatio());
        transformConstantBuffer_.SetValue({ WVP });

        RenderState::DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, indexBuffer_.GetIndexCount());

        constexpr auto WIN_FLAG =
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav;
        ImGui::SetNextWindowBgAlpha(0.2f);
        ImGui::SetNextWindowPos({ 20, 20 });
        if(ImGui::Begin("debug overlay", nullptr, WIN_FLAG))
        {
            ImGui::Text("FPS: %i\n", fps_.fps());
        }
        ImGui::End();

        window_.ImGuiRender();

        window_.SwapBuffers();
        fps_.frame_end();
    }
};

int main()
{
    try
    {
        Sample05_Camera app;
        app.Run();
    }
    catch(const std::exception &err)
    {
        std::cout << err.what() << std::endl;
        return 1;
    }
}
