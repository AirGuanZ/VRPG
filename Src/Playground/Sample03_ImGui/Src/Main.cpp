#include <iostream>

#include <agz/utility/misc.h>

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_dx11.h>
#include <ImGui/imgui_impl_win32.h>
#include <ImGui/input_dispatcher.h>

#include <SampleApp.h>

using namespace VRPG::Base;
using namespace D3D;

class ImGuiInputDispatcher :
    public EventHandler<RawKeyDownEvent>,
    public EventHandler<RawKeyUpEvent>,
    public EventHandler<CharInputEvent>,
    public EventHandler<MouseButtonDownEvent>,
    public EventHandler<MouseButtonUpEvent>,
    public EventHandler<WheelScrollEvent>
{
    static int ToImGuiMouseButton(MouseButton mb) noexcept
    {
        if(mb == MouseButton::Left)
            return 0;
        if(mb == MouseButton::Right)
            return 1;
        return 2;
    }

public:

    void Handle(const RawKeyDownEvent &e) override
    {
        ImGui::Input::KeyDown(e.vk);
    }

    void Handle(const RawKeyUpEvent &e) override
    {
        ImGui::Input::KeyUp(e.vk);
    }

    void Handle(const CharInputEvent &e) override
    {
        ImGui::Input::Char(e.ch);
    }

    void Handle(const MouseButtonDownEvent &e) override
    {
        ImGui::Input::MouseButtonDown(ToImGuiMouseButton(e.button));
    }

    void Handle(const MouseButtonUpEvent &e) override
    {
        ImGui::Input::MouseButtonUp(ToImGuiMouseButton(e.button));
    }

    void Handle(const WheelScrollEvent &e) override
    {
        ImGui::Input::MouseWheel(e.offset);
    }
};

class Sample03_ImGui: public SampleApp
{
    ImGuiInputDispatcher imguiInputDispatcher_;

protected:

    WindowDesc GetWindowDesc() override
    {
        WindowDesc desc;
        desc.windowTitle = L"Sample03 ImGui";
        return desc;
    }

    void Initialize() override
    {
        ImGui::CreateContext();
        ImGui_ImplWin32_Init(window_.GetNativeWindowHandle());
        ImGui_ImplDX11_Init(gDevice, gDeviceContext);
        
        keyboard_.Attach(static_cast<EventHandler<RawKeyDownEvent>     *>(&imguiInputDispatcher_));
        keyboard_.Attach(static_cast<EventHandler<RawKeyUpEvent>       *>(&imguiInputDispatcher_));
        keyboard_.Attach(static_cast<EventHandler<CharInputEvent>      *>(&imguiInputDispatcher_));
        mouse_.Attach   (static_cast<EventHandler<MouseButtonDownEvent>*>(&imguiInputDispatcher_));
        mouse_.Attach   (static_cast<EventHandler<MouseButtonUpEvent>  *>(&imguiInputDispatcher_));
        mouse_.Attach   (static_cast<EventHandler<WheelScrollEvent>    *>(&imguiInputDispatcher_));
    }

    void Frame() override
    {
        window_.DoEvents();
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if(keyboard_.IsKeyPressed(KEY_ESCAPE))
            window_.SetCloseFlag(true);

        static float backgroundColor[] = { 0.2f, 0.2f, 0.2f, 0 };
        window_.ClearDefaultDepthStencil();
        window_.ClearDefaultRenderTarget(backgroundColor);

        if(ImGui::Begin("ImGui", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Button("I'm a button");
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        window_.SwapBuffers();
    }

    void Destroy() override
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
};

int main()
{
    try
    {
        Sample03_ImGui app;
        app.Run();
    }
    catch(const std::exception &err)
    {
        std::cout << err.what() << std::endl;
        return 1;
    }
}
