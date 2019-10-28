#pragma once

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_dx11.h>
#include <ImGui/imgui_impl_win32.h>
#include <ImGui/input_dispatcher.h>

#include <VRPG/Base/Base.h>

class SampleApp
{
protected:

    VRPG::Base::Window window_;
    VRPG::Base::KeyboardEventManager *keyboard_ = nullptr;
    VRPG::Base::MouseEventManager *mouse_ = nullptr;

    virtual VRPG::Base::WindowDesc GetWindowDesc()
    {
        VRPG::Base::WindowDesc desc;
        desc.windowTitle = L"SampleApp";
        return desc;
    }

    virtual void Initialize()
    {

    }

    virtual void Frame()
    {

    }

    virtual void Destroy()
    {

    }

public:

    virtual ~SampleApp() = default;

    void Run()
    {
        VRPG::Base::WindowDesc windowDesc = GetWindowDesc();
        window_.Initialize(windowDesc);
        mouse_ = window_.GetMouse();
        keyboard_ = window_.GetKeyboard();

        Initialize();
        while(!window_.GetCloseFlag())
        {
            Frame();
        }
        Destroy();
    }
};
