#include "KeyEvent.h"
#include <sstream>

KeyEvent::KeyEvent(uint8 keycode)
	:Keycode(keycode)
{
}

KeyPressedEvent::KeyPressedEvent(uint8 keycode)
	:KeyEvent(keycode)
{
}

EventType KeyPressedEvent::GetEventTypeStatic()
{
	return EventType::KeyPressed;
}

EventType KeyPressedEvent::GetEventType() const
{
	return EventType::KeyPressed;
}

constexpr const char* KeyPressedEvent::GetName() const
{
	return "KeyPressedEvent";
}

std::string KeyPressedEvent::GetEventInfo() const
{
	std::stringstream ss;
	ss << GetName() << "| Keycode: " << Keycode;
	return ss.str();
}

KeyReleasedEvent::KeyReleasedEvent(uint8 keycode)
	:KeyEvent(keycode)
{
}

EventType KeyReleasedEvent::GetEventTypeStatic()
{
	return EventType::KeyReleased;
}

EventType KeyReleasedEvent::GetEventType() const
{
	return EventType::KeyReleased;
}

constexpr const char* KeyReleasedEvent::GetName() const
{
	return "KeyReleasedEvent";
}

std::string KeyReleasedEvent::GetEventInfo() const
{
	std::stringstream ss;
	ss << GetName() << "| Keycode: " << Keycode;
	return ss.str();
}

KeyTypedEvent::KeyTypedEvent(uint8 keycode)
	:KeyEvent(keycode)
{
}

EventType KeyTypedEvent::GetEventTypeStatic()
{
	return EventType::KeyTyped;
}

EventType KeyTypedEvent::GetEventType() const
{
	return EventType::KeyTyped;
}

constexpr const char* KeyTypedEvent::GetName() const
{
	return "KeyTypedEvent";
}

std::string KeyTypedEvent::GetEventInfo() const
{
	std::stringstream ss;
	ss << GetName() << "| Keycode: " << Keycode;
	return ss.str();
}
