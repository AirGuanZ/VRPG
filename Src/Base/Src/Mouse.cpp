#include <Windows.h>

#include <VRPG/Base/Mouse.h>
#include <VRPG/Base/Window.h>

VRPG_BASE_BEGIN

void MouseEventManager::SetCursorLock(bool locked, int lockPositionX, int lockPositionY)
{
    isCursorLocked_ = locked;
    lockPositionX_ = lockPositionX;
    lockPositionY_ = lockPositionY;
}

void MouseEventManager::ShowCursor(bool show)
{
    showCursor_ = show;
    ::ShowCursor(show ? TRUE : FALSE);
}

void MouseEventManager::Update()
{
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    ScreenToClient(hWindow_, &cursorPos);

    relativeCursorX_ = int(cursorPos.x - cursorX_);
    relativeCursorY_ = int(cursorPos.y - cursorY_);
    cursorX_ = int(cursorPos.x);
    cursorY_ = int(cursorPos.y);

    if(relativeCursorX_ || relativeCursorY_)
    {
        EventManager::InvokeAllHandlers(CursorMoveEvent{
        cursorX_, cursorY_, relativeCursorX_, relativeCursorY_ });
    }

    if(isCursorLocked_)
    {
        POINT lockPos = { static_cast<LONG>(lockPositionX_), static_cast<LONG>(lockPositionY_) };
        ClientToScreen(hWindow_, &lockPos);
        SetCursorPos(lockPos.x, lockPos.y);

        GetCursorPos(&cursorPos);
        ScreenToClient(hWindow_, &cursorPos);
        cursorX_ = int(cursorPos.x);
        cursorY_ = int(cursorPos.y);
    }

    for(int i = 0; i < 3; ++i)
    {
        isButtonDown_[i] = !isButtonPressedLastFrame_[i] && isButtonPressed_[i];
        isButtonUp_[i]   = isButtonPressedLastFrame_[i] && !isButtonPressed_[i];
        isButtonPressedLastFrame_[i] = isButtonPressed_[i];
    }
}

void MouseEventManager::ClearState()
{
    isButtonPressedLastFrame_[0] = isButtonPressedLastFrame_[1] = isButtonPressedLastFrame_[2] = false;
    isButtonPressed_[0]          = isButtonPressed_[1]          = isButtonPressed_[2]          = false;
    isButtonDown_[0]             = isButtonDown_[1]             = isButtonDown_[2]             = false;
    isButtonUp_[0]               = isButtonUp_[1]               = isButtonUp_[2]               = false;
    
    cursorX_         = cursorY_         = 0;
    lastCursorX_     = lastCursorY_     = 0;
    relativeCursorX_ = relativeCursorY_ = 0;

    isCursorLocked_ = false;
    if(!showCursor_)
    {
        ShowCursor(true);
    }
}
VRPG_BASE_END
