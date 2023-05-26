#pragma once
#include "Event.h"

class WindowEvent : public Event
{
	virtual EventCategory GetCategory() const override;
};

class WindowResizeEvent : public WindowEvent
{
public:
	WindowResizeEvent(uint32_t width, uint32_t height);

	inline uint32_t GetWidth() const { return Width; }
	inline uint32_t GetHeight() const { return Height; }

	static EventType GetEventTypeStatic();
	virtual EventType GetEventType() const override;
	virtual constexpr const char* GetName() const override;
	virtual std::string GetEventInfo() const override;

private:
	uint32_t Width, Height;
};

class WindowCloseEvent : public WindowEvent
{
public:
	WindowCloseEvent() = default;

	static EventType GetEventTypeStatic();
	virtual EventType GetEventType() const override;
	virtual constexpr const char* GetName() const override;
	virtual std::string GetEventInfo() const override;
};

class WindowLostFocusEvent : public WindowEvent
{
public:
	WindowLostFocusEvent() = default;

	static EventType GetEventTypeStatic();
	virtual EventType GetEventType() const override;
	virtual constexpr const char* GetName() const override;
	virtual std::string GetEventInfo() const override;
};