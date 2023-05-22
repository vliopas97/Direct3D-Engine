#include "MouseEvent.h"
#include <sstream>

MouseButtonEvent::MouseButtonEvent(MouseButtonCode button)
	:Button(button)
{
}

MouseButtonPressedEvent::MouseButtonPressedEvent(MouseButtonCode button)
	:MouseButtonEvent(button)
{
}

EventType MouseButtonPressedEvent::GetEventTypeStatic()
{
	return EventType::MouseButtonPressed;
}

EventType MouseButtonPressedEvent::GetEventType() const
{
	return EventType::MouseButtonPressed;
}

constexpr const char* MouseButtonPressedEvent::GetName() const
{
	return "MouseButtonPressedEvent";
}

std::string MouseButtonPressedEvent::GetEventInfo() const
{
	std::stringstream ss;
	ss << GetName() << "| MouseButtonCode: " << static_cast<int>(Button);
	return ss.str();
}

MouseButtonReleasedEvent::MouseButtonReleasedEvent(MouseButtonCode button)
	:MouseButtonEvent(button)
{
}

EventType MouseButtonReleasedEvent::GetEventTypeStatic()
{
	return EventType::MouseButtonReleased;
}

EventType MouseButtonReleasedEvent::GetEventType() const
{
	return EventType::MouseButtonReleased;
}

constexpr const char* MouseButtonReleasedEvent::GetName() const
{
	return "MouseButtonReleasedEvent";
}

std::string MouseButtonReleasedEvent::GetEventInfo() const
{
	std::stringstream ss;
	ss << GetName() << "| MouseButtonCode: " << static_cast<int>(Button);
	return ss.str();
}

MouseMovedEvent::MouseMovedEvent(uint32_t x, uint32_t y)
	:XPos(x), YPos(y)
{
}

EventType MouseMovedEvent::GetEventTypeStatic()
{
	return EventType::MouseMoved;
}

EventType MouseMovedEvent::GetEventType() const
{
	return EventType::MouseMoved;
}

constexpr const char* MouseMovedEvent::GetName() const
{
	return "MouseMovedEvent";
}

std::string MouseMovedEvent::GetEventInfo() const
{
	return MouseMovedEvent::GetName();
}

MouseScrolledEvent::MouseScrolledEvent(uint32_t xOffset, uint32_t yOffset)
	:XOffset(xOffset), YOffset(yOffset)
{
}

EventType MouseScrolledEvent::GetEventTypeStatic()
{
	return EventType::MouseScrolled;
}

EventType MouseScrolledEvent::GetEventType() const
{
	return EventType::MouseScrolled;
}

constexpr const char* MouseScrolledEvent::GetName() const
{
	return "MouseScrolledEvent";
}

std::string MouseScrolledEvent::GetEventInfo() const
{
	return MouseScrolledEvent::GetName();
}

EventType MouseEnterEvent::GetEventTypeStatic()
{
	return EventType::MouseEnter;
}

EventType MouseEnterEvent::GetEventType() const
{
	return EventType::MouseEnter;
}

constexpr const char* MouseEnterEvent::GetName() const
{
	return "MouseEnterEvent";
}

std::string MouseEnterEvent::GetEventInfo() const
{
	return MouseEnterEvent::GetName();
}

EventType MouseLeaveEvent::GetEventTypeStatic()
{
	return EventType::MouseLeave;
}

EventType MouseLeaveEvent::GetEventType() const
{
	return EventType::MouseLeave;
}

constexpr const char* MouseLeaveEvent::GetName() const
{
	return "MouseLeaveEvent";
}

std::string MouseLeaveEvent::GetEventInfo() const
{
	return MouseLeaveEvent::GetName();
}