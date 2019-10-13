#pragma once

#include <VRPG/Base/Event.h>

VRPG_BASE_BEGIN

enum class MouseButton
{
    Left   = 0,
    Middle = 1,
    Right  = 2
};

struct MouseButtonDownEvent
{
    MouseButton button;
};

struct MouseButtonUpEvent
{
    MouseButton button;
};

struct CursorMoveEvent
{
    int x, y;
};

struct WheelScrollEvent
{
    int offset;
};

using MouseButtonDownHandler = FunctionalEventHandler<MouseButtonDownEvent>;
using MouseButtonUpHandler   = FunctionalEventHandler<MouseButtonUpEvent>;
using CursorMoveHandler      = FunctionalEventHandler<CursorMoveEvent>;
using WheelScrollHandler     = FunctionalEventHandler<WheelScrollEvent>;

class MouseEventManager :
    public EventManager<MouseButtonDownEvent, MouseButtonUpEvent, CursorMoveEvent, WheelScrollEvent>
{
    bool isButtonPressed_[3] = { false, false, false };
    int cursorX_         = 0, cursorY_         = 0;
    int lastCursorX_     = 0, lastCursorY_     = 0;
    int relativeCursorX_ = 0, relativeCursorY_ = 0;

public:

    void InvokeAllHandlers(const MouseButtonDownEvent &e)
    {
        isButtonPressed_[int(e.button)] = true;
        EventManager::InvokeAllHandlers(e);
    }

    void InvokeAllHandlers(const MouseButtonUpEvent &e)
    {
        isButtonPressed_[int(e.button)] = false;
        EventManager::InvokeAllHandlers(e);
    }

    void InvokeAllHandlers(const CursorMoveEvent &e)
    {
        cursorX_ = e.x;
        cursorY_ = e.y;
        EventManager::InvokeAllHandlers(e);
    }

    void InvokeAllHandlers(const WheelScrollEvent &e)
    {
        EventManager::InvokeAllHandlers(e);
    }

    bool IsMouseButtonPressed(MouseButton button) const noexcept
    {
        return isButtonPressed_[int(button)];
    }

    int GetCursorPositionX() const noexcept
    {
        return cursorX_;
    }

    int GetCursorPositionY() const noexcept
    {
        return cursorY_;
    }

    int GetRelativeCursorPositionX() const noexcept
    {
        return relativeCursorX_;
    }

    int GetRelativeCursorPositionY() const noexcept
    {
        return relativeCursorY_;
    }

    void Update()
    {
        relativeCursorX_ = cursorX_ - lastCursorX_;
        relativeCursorY_ = cursorY_ - lastCursorY_;
        lastCursorX_ = cursorX_;
        lastCursorY_ = cursorY_;
    }
};

VRPG_BASE_END
