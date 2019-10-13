#pragma once

#include <VRPG/Base/Event.h>
#include <VRPG/Base/KeyCode.h>

VRPG_BASE_BEGIN

struct KeyDownEvent
{
    KeyCode key;
};

struct KeyUpEvent
{
    KeyCode key;
};

struct CharInputEvent
{
    uint32_t ch;
};

class KeyboardEventManager
    : public EventManager<KeyDownEvent, KeyUpEvent, CharInputEvent>
{
    bool isKeyPressed_[KEY_MAX + 1] = { false };

    void UpdateSingleKey(bool pressed, KeyCode keycode) noexcept
    {
        if(pressed && !IsKeyPressed(keycode))
            InvokeAllHandlers(KeyDownEvent{ keycode });
        else if(!pressed && IsKeyPressed(keycode))
            InvokeAllHandlers(KeyUpEvent{ keycode });
    }

public:

    void InvokeAllHandlers(const KeyDownEvent &e)
    {
        isKeyPressed_[e.key] = true;
        EventManager::InvokeAllHandlers(e);
    }

    void InvokeAllHandlers(const KeyUpEvent &e)
    {
        isKeyPressed_[e.key] = false;
        EventManager::InvokeAllHandlers(e);
    }

    void InvokeAllHandlers(const CharInputEvent &e)
    {
        EventManager::InvokeAllHandlers(e);
    }

    bool IsKeyPressed(KeyCode key) const noexcept
    {
        return isKeyPressed_[key];
    }

    void Update()
    {
        bool leftShiftPressed  = (GetKeyState(VK_LSHIFT)   & 0x8000) != 0;
        bool rightShiftPressed = (GetKeyState(VK_RSHIFT)   & 0x8000) != 0;
        bool leftCtrlPressed   = (GetKeyState(VK_LCONTROL) & 0x8000) != 0;
        bool rightCtrlPressed  = (GetKeyState(VK_RCONTROL) & 0x8000) != 0;
        bool leftAltPressed    = (GetKeyState(VK_LMENU)    & 0x8000) != 0;
        bool rightAltPressed   = (GetKeyState(VK_RMENU)    & 0x8000) != 0;

        UpdateSingleKey(leftShiftPressed,  KEY_LSHIFT);
        UpdateSingleKey(rightShiftPressed, KEY_LSHIFT);
        UpdateSingleKey(leftCtrlPressed,   KEY_LCTRL);
        UpdateSingleKey(rightCtrlPressed,  KEY_RCTRL);
        UpdateSingleKey(leftAltPressed,    KEY_LALT);
        UpdateSingleKey(rightAltPressed,   KEY_RALT);
    }
};

VRPG_BASE_END
