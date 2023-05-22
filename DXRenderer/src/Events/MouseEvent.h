#pragma once
#include "Event.h"

class MouseButtonEvent : public Event
{
public:
	inline uint32_t GetMouseButtonCode() const { return static_cast<uint32_t>(Button); }

protected:
	MouseButtonEvent(MouseButtonCode button);

	MouseButtonCode Button;
};

class MouseButtonPressedEvent : public MouseButtonEvent
{
public:
	MouseButtonPressedEvent(MouseButtonCode button);

	static EventType GetEventTypeStatic();
	virtual EventType GetEventType() const override;
	virtual constexpr const char* GetName() const override;
	virtual std::string GetEventInfo() const override;
};

class MouseButtonReleasedEvent : public MouseButtonEvent
{
public:
	MouseButtonReleasedEvent(MouseButtonCode button);

	static EventType GetEventTypeStatic();
	virtual EventType GetEventType() const override;
	virtual constexpr const char* GetName() const override;
	virtual std::string GetEventInfo() const override;
};

class MouseMovedEvent : public Event
{
public:
	MouseMovedEvent(uint32_t x, uint32_t y);

	inline uint32_t GetXPos() const { return XPos; }
	inline uint32_t GetYPos() const { return YPos; }

	static EventType GetEventTypeStatic();
	virtual EventType GetEventType() const override;
	virtual constexpr const char* GetName() const override;
	virtual std::string GetEventInfo() const override;

private:
	uint32_t XPos, YPos;
};

class MouseScrolledEvent : public Event
{
public:
	MouseScrolledEvent(uint32_t xOffset, uint32_t yOffset);

	inline uint32_t GetXOffset() const { return XOffset; }
	inline uint32_t GetYOffset() const { return YOffset; }

	static EventType GetEventTypeStatic();
	virtual EventType GetEventType() const override;
	virtual constexpr const char* GetName() const override;
	virtual std::string GetEventInfo() const override;

private:
	uint32_t XOffset, YOffset;
};

class MouseEnterEvent : public Event
{
public:
	MouseEnterEvent() = default;

	static EventType GetEventTypeStatic();
	virtual EventType GetEventType() const override;
	virtual constexpr const char* GetName() const override;
	virtual std::string GetEventInfo() const override;
};

class MouseLeaveEvent : public Event
{
public:
	MouseLeaveEvent() = default;

	static EventType GetEventTypeStatic();
	virtual EventType GetEventType() const override;
	virtual constexpr const char* GetName() const override;
	virtual std::string GetEventInfo() const override;
};