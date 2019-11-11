#pragma once

#include <VRPG/Game/Common.h>

VRPG_GAME_BEGIN

class MouseButtonStateTracker
{
    Base::MouseButton button_;

    bool isPressed_;
    bool isDown_;
    bool isUp_;

public:

    explicit MouseButtonStateTracker(Base::MouseButton button = Base::MouseButton::Left) noexcept
        : button_(button), isPressed_(false), isDown_(false), isUp_(false)
    {

    }

    void Update(bool pressed) noexcept
    {
        isDown_ = !isPressed_ && pressed;
        isUp_ = isPressed_ && !pressed;
        isPressed_ = pressed;
    }

    void Update(const Base::MouseEventManager *mouse) noexcept
    {
        Update(mouse->IsMouseButtonPressed(button_));
    }

    bool IsPressed() const noexcept
    {
        return isPressed_;
    }

    bool IsDown() const noexcept
    {
        return isDown_;
    }

    bool IsUp() const noexcept
    {
        return isUp_;
    }
};

class KeyStateTracker
{
    Base::KeyCode keyCode_;

    bool isPressed_;
    bool isDown_;
    bool isUp_;

public:

    explicit KeyStateTracker(Base::KeyCode keyCode = Base::KEY_SPACE) noexcept
        : keyCode_(keyCode), isPressed_(false), isDown_(false), isUp_(false)
    {

    }

    void Update(bool pressed) noexcept
    {
        isDown_ = !isPressed_ && pressed;
        isUp_   = isPressed_ && !pressed;
        isPressed_ = pressed;
    }

    void Update(const Base::KeyboardEventManager *keyboard) noexcept
    {
        Update(keyboard->IsKeyPressed(keyCode_));
    }

    bool IsPressed() const noexcept
    {
        return isPressed_;
    }

    bool IsDown() const noexcept
    {
        return isDown_;
    }

    bool IsUp() const noexcept
    {
        return isUp_;
    }
};

VRPG_GAME_END
