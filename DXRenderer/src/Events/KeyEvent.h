#pragma once
#include "Event.h"

class KeyEvent : public Event
{
public:
	virtual ~KeyEvent() = default;
	inline int GetKeycode() const
	{
		return static_cast<int>(Keycode);
	}
	
protected:
	KeyEvent(uint8 keycode);

	uint8 Keycode;
};

class KeyPressedEvent : public KeyEvent
{
public:
	KeyPressedEvent(uint8 keycode);

	static EventType GetEventTypeStatic();
	virtual EventType GetEventType() const override;
	virtual constexpr const char* GetName() const override;
	virtual std::string GetEventInfo() const override;
};

class KeyReleasedEvent : public KeyEvent
{
public:
	KeyReleasedEvent(uint8 keycode);

	static EventType GetEventTypeStatic();
	virtual EventType GetEventType() const override;
	virtual constexpr const char* GetName() const override;
	virtual std::string GetEventInfo() const override;
};

class KeyTypedEvent : public KeyEvent
{
public:
	KeyTypedEvent(uint8 keycode);

	static EventType GetEventTypeStatic();
	virtual EventType GetEventType() const override;
	virtual constexpr const char* GetName() const override;
	virtual std::string GetEventInfo() const override;
};