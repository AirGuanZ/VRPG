#include <iostream>

#include <SampleApp.h>

using namespace VRPG::Base;

class Sample01_Triangle : public SampleApp
{
protected:

    WindowResizeHandler    windowResizeHandler_;
    KeyDownHandler         keyDownHandler_;
    MouseButtonDownHandler mouseButtonDownHandler_;
    WheelScrollHandler     wheelScrollHandler_;

    WindowDesc GetWindowDesc() override
    {
        WindowDesc desc;
        desc.windowTitle = L"Sample01 Window";
        return desc;
    }

    void Initialize() override
    {
        windowResizeHandler_.SetFunction(
            [&](const WindowResizeEvent &e)
        {
            std::cout << "resize window to (" << e.newClientWidth << ", " << e.newClientHeight << ")" << std::endl;
        });
        window_.Attach(&windowResizeHandler_);

        keyDownHandler_.SetFunction(
            [&](const KeyDownEvent &e)
        {
            std::cout << "key down: " << e.key << std::endl;
        });
        keyboard_->Attach(&keyDownHandler_);

        mouseButtonDownHandler_.SetFunction([&](const MouseButtonDownEvent &e)
        {
            std::cout << "mouse button " << int(e.button) << " clicked" << std::endl;
        });
        mouse_->Attach(&mouseButtonDownHandler_);

        wheelScrollHandler_.SetFunction([&](const WheelScrollEvent &e)
        {
            std::cout << "wheel scroll offset: " << e.offset << std::endl;
        });
        mouse_->Attach(&wheelScrollHandler_);
    }

    void Frame() override
    {
        window_.DoEvents();

        if(keyboard_->IsKeyPressed(KEY_ESCAPE))
            window_.SetCloseFlag(true);
        window_.ClearDefaultRenderTarget();

        window_.SwapBuffers();
    }
};

int main()
{
    try
    {
        Sample01_Triangle app;
        app.Run();
    }
    catch(const std::exception &err)
    {
        std::cout << err.what() << std::endl;
        return 1;
    }
}
