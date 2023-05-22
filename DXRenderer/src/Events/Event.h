#pragma once
#include "Core/Core.h"

#include <string>
#include <functional>

using uint8 = unsigned char;

enum class EventType
{
    None = 0,
    WindowResize, WindowClose, WindowLostFocus,
    KeyPressed, KeyReleased, KeyTyped,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled, MouseEnter, MouseLeave

};

enum class MouseButtonCode : uint16_t
{
    ButtonLeft = VK_LBUTTON,
    ButtonMiddle = VK_MBUTTON,
    ButtonRight = VK_RBUTTON,
    ButtonExtended1 = VK_XBUTTON1,
    ButtonExtended2 = VK_XBUTTON2
};

class Event
{
public:
    virtual EventType GetEventType() const = 0;
    virtual constexpr const char* GetName() const = 0;
    virtual std::string GetEventInfo() const = 0;

    bool Handled = false;
};

class EventDispatcher
{
    template<typename T>
    using EventFn = std::function<bool(T&)>;

public:
    EventDispatcher(Event& e)
        : DispatchedEvent(e)
    {
    }

    template<typename T>
    bool Dispatch(EventFn<T> func)
    {
        auto eventType = DispatchedEvent.GetEventType();
        if (eventType == T::GetEventTypeStatic())
        {
            DispatchedEvent.Handled = func(static_cast<T&>(DispatchedEvent));
            return true;
        }
        return false;
    }

private:
    Event& DispatchedEvent;
};
