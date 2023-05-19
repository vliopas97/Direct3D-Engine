#pragma once
#include "Core/Core.h"

#include <string>
#include <functional>

using uint8 = unsigned char;

enum class EventType
{
    None = 0,
    KeyPressed = WM_KEYDOWN, KeyReleased = WM_KEYUP, KeyTyped = WM_CHAR
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
