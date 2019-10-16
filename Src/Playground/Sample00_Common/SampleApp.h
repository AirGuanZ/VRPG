#pragma once

#include <VRPG/Base/Base.h>

class SampleApp
{
protected:

    VRPG::Base::Window window_;
    VRPG::Base::KeyboardEventManager keyboard_;
    VRPG::Base::MouseEventManager mouse_;

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
        window_.SetMouse(&mouse_);
        window_.SetKeyboard(&keyboard_);

        Initialize();
        while(!window_.GetCloseFlag())
            Frame();
        Destroy();
    }
};
