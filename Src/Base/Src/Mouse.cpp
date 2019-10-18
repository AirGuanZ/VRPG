#include <Windows.h>

#include <VRPG/Base/Mouse.h>
#include <VRPG/Base/Window.h>

VRPG_BASE_BEGIN

void MouseEventManager::SetCursorLock(bool locked, int lockPositionX, int lockPositionY)
{
    isCursorLocked_ = locked;

    POINT pnt = { lockPositionX, lockPositionY };
    ClientToScreen(hWindow_, &pnt);
    lockPositionX_ = int(pnt.x);
    lockPositionY_ = int(pnt.y);
}

void MouseEventManager::ShowCursor(bool show)
{
    showCursor_ = show;
    ::ShowCursor(show ? TRUE : FALSE);
}

void MouseEventManager::UpdatePosition()
{
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    ScreenToClient(hWindow_, &cursorPos);

    relativeCursorX_ = int(cursorPos.x - cursorX_);
    relativeCursorY_ = int(cursorPos.y - cursorY_);
    cursorX_ = int(cursorPos.x);
    cursorY_ = int(cursorPos.y);
    if(relativeCursorX_ || relativeCursorY_)
        EventManager::InvokeAllHandlers(CursorMoveEvent{
        cursorX_, cursorY_, relativeCursorX_, relativeCursorY_ });

    if(isCursorLocked_)
    {
        SetCursorPos(lockPositionX_, lockPositionY_);

        GetCursorPos(&cursorPos);
        ScreenToClient(hWindow_, &cursorPos);
        cursorX_ = int(cursorPos.x);
        cursorY_ = int(cursorPos.y);
    }
}

VRPG_BASE_END
