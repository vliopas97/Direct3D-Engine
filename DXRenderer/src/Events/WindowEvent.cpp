#include "WindowEvent.h"
#include <sstream>

WindowResizeEvent::WindowResizeEvent(uint32_t width, uint32_t height)
	:Width(width), Height(height)
{
}

EventType WindowResizeEvent::GetEventTypeStatic()
{
	return EventType::WindowResize;
}

EventType WindowResizeEvent::GetEventType() const
{
	return EventType::WindowResize;
}

constexpr const char* WindowResizeEvent::GetName() const
{
	return "WindowResizeEvent";
}

std::string WindowResizeEvent::GetEventInfo() const
{
	std::stringstream ss;
	ss << GetName() << "| Width: " << Width <<", Height: " << Height;
	return ss.str();
}

EventType WindowCloseEvent::GetEventTypeStatic()
{
	return EventType::WindowClose;
}

EventType WindowCloseEvent::GetEventType() const
{
	return EventType::WindowClose;
}

constexpr const char* WindowCloseEvent::GetName() const
{
	return "WindowCloseEvent";
}

std::string WindowCloseEvent::GetEventInfo() const
{
	return WindowCloseEvent::GetName();
}

EventType WindowLostFocusEvent::GetEventTypeStatic()
{
	return EventType::WindowLostFocus;
}

EventType WindowLostFocusEvent::GetEventType() const
{
	return EventType::WindowLostFocus;
}

constexpr const char* WindowLostFocusEvent::GetName() const
{
	return "WindowLostFocusEvent";
}

std::string WindowLostFocusEvent::GetEventInfo() const
{
	return WindowLostFocusEvent::GetName();
}
