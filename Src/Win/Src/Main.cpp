#include <iostream>

#include <VRPG/Win/Window.h>

void run()
{
    using namespace vrpg::win;

    Window window;
    window.Initialize(WindowDesc{});

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
        window.ClearRenderTarget();
        window.SwapBuffers();

        window.DoEvents();

        if(keyboard.IsKeyPressed(KEY_ESCAPE))
            window.SetCloseFlag(true);
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
