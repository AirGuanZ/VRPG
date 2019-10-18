#include <iostream>

#include <VRPG/Base/Base.h>

void Run()
{
    using namespace VRPG::Base;

    Window window;
    WindowDesc desc;
    window.Initialize(desc);

    KeyboardEventManager *keyboard = window.GetKeyboard();

    while(!window.GetCloseFlag())
    {
        window.DoEvents();

        if(keyboard->IsKeyPressed(KEY_ESCAPE))
            window.SetCloseFlag(true);

        window.ClearDefaultRenderTarget();
        window.SwapBuffers();
    }
}

int main()
{
    try
    {
        Run();
    }
    catch(const std::exception &err)
    {
        std::cout << err.what() << std::endl;
        return 1;
    }
}
