#include <iostream>

#include <VRPG/Base/Window.h>

#include <VRPG/Base/D3D/Shader/Shader.h>

void run()
{
    using namespace VRPG::Base;

    Window window;
    WindowDesc desc;
    window.Initialize(desc);

    MouseEventManager mouse;
    window.SetMouse(&mouse);

    KeyboardEventManager keyboard;
    window.SetKeyboard(&keyboard);

    WindowResizeHandler resizeHandler([&](const WindowResizeEvent &e)
    {
        std::cout << "(" << e.newClientWidth << ", " << e.newClientHeight << ")" << std::endl;
    });
    window.Attach(&resizeHandler);

    WindowGetFocusHandler getFocusHandler([&](const WindowGetFocusEvent &)
    {
        std::cout << "get focus" << std::endl;
    });
    window.Attach(&getFocusHandler);

    WindowLostFocusHandler lostFocusHandler([&](const WindowLostFocusEvent &)
    {
        std::cout << "lost focus" << std::endl;
    });
    window.Attach(&lostFocusHandler);

    CursorMoveHandler cursorMoveHandler([&](const CursorMoveEvent &e)
    {
        std::cout << "[" << e.x << ", " << e.y << "]" << std::endl;
    });
    mouse.Attach(&cursorMoveHandler);

    while(!window.GetCloseFlag())
    {
        window.DoEvents();

        if(keyboard.IsKeyPressed(KEY_ESCAPE))
            window.SetCloseFlag(true);

        window.ClearRenderTarget();
        window.SwapBuffers();
    }
}

int main()
{
    try
    {
        run();
    }
    catch(const std::exception &err)
    {
        std::cout << err.what() << std::endl;
        return 1;
    }
}
