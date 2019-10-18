#pragma once

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_dx11.h>
#include <ImGui/imgui_impl_win32.h>
#include <ImGui/input_dispatcher.h>

#include <VRPG/Base/Base.h>

class SampleApp
{
    class ImGuiInputDispatcher :
        public VRPG::Base::EventHandler<VRPG::Base::RawKeyDownEvent>,
        public VRPG::Base::EventHandler<VRPG::Base::RawKeyUpEvent>,
        public VRPG::Base::EventHandler<VRPG::Base::CharInputEvent>,
        public VRPG::Base::EventHandler<VRPG::Base::MouseButtonDownEvent>,
        public VRPG::Base::EventHandler<VRPG::Base::MouseButtonUpEvent>,
        public VRPG::Base::EventHandler<VRPG::Base::WheelScrollEvent>
    {
        static int ToImGuiMouseButton(VRPG::Base::MouseButton mb) noexcept
        {
            if(mb == VRPG::Base::MouseButton::Left)
                return 0;
            if(mb == VRPG::Base::MouseButton::Right)
                return 1;
            return 2;
        }

    public:

        void Handle(const VRPG::Base::RawKeyDownEvent &e) override
        {
            ImGui::Input::KeyDown(e.vk);
        }

        void Handle(const VRPG::Base::RawKeyUpEvent &e) override
        {
            ImGui::Input::KeyUp(e.vk);
        }

        void Handle(const VRPG::Base::CharInputEvent &e) override
        {
            ImGui::Input::Char(e.ch);
        }

        void Handle(const VRPG::Base::MouseButtonDownEvent &e) override
        {
            ImGui::Input::MouseButtonDown(ToImGuiMouseButton(e.button));
        }

        void Handle(const VRPG::Base::MouseButtonUpEvent &e) override
        {
            ImGui::Input::MouseButtonUp(ToImGuiMouseButton(e.button));
        }

        void Handle(const VRPG::Base::WheelScrollEvent &e) override
        {
            ImGui::Input::MouseWheel(e.offset);
        }

        void AttachTo(VRPG::Base::KeyboardEventManager &keyboard, VRPG::Base::MouseEventManager &mouse)
        {
            keyboard.Attach(static_cast<EventHandler<VRPG::Base::RawKeyDownEvent> *>(this));
            keyboard.Attach(static_cast<EventHandler<VRPG::Base::RawKeyUpEvent> *>(this));
            keyboard.Attach(static_cast<EventHandler<VRPG::Base::CharInputEvent> *>(this));

            mouse.Attach(static_cast<EventHandler<VRPG::Base::MouseButtonDownEvent> *>(this));
            mouse.Attach(static_cast<EventHandler<VRPG::Base::MouseButtonUpEvent> *>(this));
            mouse.Attach(static_cast<EventHandler<VRPG::Base::WheelScrollEvent> *>(this));
        }
    };

    ImGuiInputDispatcher imguiInputDispatcher_;

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

        ImGui::CreateContext();
        ImGui_ImplWin32_Init(window_.GetNativeWindowHandle());
        ImGui_ImplDX11_Init(VRPG::Base::gDevice, VRPG::Base::gDeviceContext);
        imguiInputDispatcher_.AttachTo(*keyboard_, *mouse_);

        Initialize();
        while(!window_.GetCloseFlag())
        {
            Frame();
        }

        Destroy();
    }
};
