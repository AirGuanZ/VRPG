#include <iostream>

#include <agz/utility/misc.h>

#include <SampleApp.h>

using namespace VRPG::Base;
using namespace D3D;

class Sample03_ImGui : public SampleApp
{
protected:

    WindowDesc GetWindowDesc() override
    {
        WindowDesc desc;
        desc.windowTitle = L"Sample03 ImGui";
        return desc;
    }

    void Frame() override
    {
        window_.DoEvents();
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if(keyboard_->IsKeyPressed(KEY_ESCAPE))
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
