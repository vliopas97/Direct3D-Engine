#include "Input.h"

#include <imgui.h>
#include <typeinfo>
#include <unordered_map>

#define BIND_EVENT_FN(x) std::bind(&InputManager::x, this, std::placeholders::_1)

class EventFactory
{
	std::unordered_map<EventCategory, UniquePtr<EventFactoryBase>> EventFactories;
public:
	EventFactory()
	{
		EventFactories[EventCategory::MouseEvents] = MakeUnique<MouseEventFactory>();
		EventFactories[EventCategory::KeyEvents] = MakeUnique<KeyEventFactory>();
	}

	UniquePtr<Event> Make(Event* e)
	{
		return EventFactories[e->GetCategory()]->Make(e);
	}
};

void InputManager::FlushKeyEventBuffer() noexcept
{
	KeyEventBuffer = std::queue<UniquePtr<Event>>();
}

std::optional<UniquePtr<Event>> InputManager::FetchKeyEvent() noexcept
{
	if (KeyEventBuffer.size() > 0)
	{
		auto eventPtr = KeyEventBuffer.front().release();
		KeyEventBuffer.pop();
		EventFactory eventGenerator;
		return std::optional<UniquePtr<Event>>(eventGenerator.Make(eventPtr));
	}
	else
		return std::nullopt;
}

std::optional<UniquePtr<Event>> InputManager::FetchMouseEvent() noexcept
{
	if (MouseEventBuffer.size() > 0)
	{
		auto eventPtr = MouseEventBuffer.front().release();
		MouseEventBuffer.pop();
		EventFactory eventGenerator;
		return std::optional<UniquePtr<Event>>(eventGenerator.Make(eventPtr));
	}
	else
		return std::nullopt;
}

void InputManager::FlushCharBuffer() noexcept
{
	CharBuffer = std::queue<uint8>();
}

uint8 InputManager::FetchKeyTyped() noexcept
{
	if (CharBuffer.size() > 0)
	{
		uint8 charCode = CharBuffer.front();
		CharBuffer.pop();
		return charCode;
	}
	else
		return 0;
}

void InputManager::ResetKeyStates() noexcept
{
	KeyStates.reset();
}

void InputManager::FlushAll() noexcept
{
	FlushKeyEventBuffer();
	FlushCharBuffer();
}

void InputManager::SetAutoRepeat(bool autoRepeat) noexcept
{
	RepeatEnabled = autoRepeat;
}

bool InputManager::IsKeyPressed(uint8 keycode) const noexcept
{
	return KeyStates[keycode];
}

bool InputManager::IsKeyBufferEmpty() const noexcept
{
	return KeyEventBuffer.empty();
}

bool InputManager::IsMouseBufferEmpty() const noexcept
{
	return MouseEventBuffer.empty();
}

bool InputManager::IsAutoRepeatEnabled() const noexcept
{
	return RepeatEnabled;
}

bool InputManager::IsMouseInWindow() const noexcept
{
	return MouseInWindow;
}

bool InputManager::IsCharBufferEmpty() noexcept
{
	return CharBuffer.empty();
}

void InputManager::HideCursor()
{
	CursorVisibility = false;
	while (::ShowCursor(FALSE) >= 0);
}

void InputManager::ShowCursor()
{
	CursorVisibility = true;
	while (::ShowCursor(TRUE) < 0);
}

bool InputManager::IsCursorVisible() const
{
	return CursorVisibility;
}

void InputManager::OnEvent(Event& event)
{
	EventDispatcher dispatcher(event);

	dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(OnKeyPressed));
	dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN(OnKeyReleased));
	dispatcher.Dispatch<KeyTypedEvent>(BIND_EVENT_FN(OnKeyTyped));

	dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(OnMouseButtonPressed));
	dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN(OnMouseButtonReleased));
	dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(OnMouseMoved));
	dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(OnMouseScrolled));
	dispatcher.Dispatch<MouseEnterEvent>(BIND_EVENT_FN(OnMouseEnter));
	dispatcher.Dispatch<MouseLeaveEvent>(BIND_EVENT_FN(OnMouseLeave));
}

bool InputManager::OnKeyPressed(KeyPressedEvent& event) noexcept
{
	if (!IsAutoRepeatEnabled() && event.IsRepeated())
		return true;

	KeyStates[event.GetKeycode()] = true;
	KeyEventBuffer.emplace(MakeUnique<KeyPressedEvent>(event.GetKeycode(), event.IsRepeated()));
	PreventBufferOverflow(KeyEventBuffer);
	return true;
}

bool InputManager::OnKeyReleased(KeyReleasedEvent& event) noexcept
{
	KeyStates[event.GetKeycode()] = false;
	KeyEventBuffer.emplace(MakeUnique<KeyReleasedEvent>(event.GetKeycode()));
	PreventBufferOverflow(KeyEventBuffer);
	return true;
}

bool InputManager::OnKeyTyped(KeyTypedEvent& event) noexcept
{
	CharBuffer.push(event.GetKeycode());
	PreventBufferOverflow(KeyEventBuffer);
	return true;
}

bool InputManager::OnMouseButtonPressed(MouseButtonPressedEvent& event) noexcept
{
	MouseStates[event.GetMouseButtonCode()] = true;
	MouseEventBuffer.emplace(MakeUnique<MouseButtonPressedEvent>(static_cast<MouseButtonCode>(event.GetMouseButtonCode())));
	PreventBufferOverflow(MouseEventBuffer);
	return true;
}

bool InputManager::OnMouseButtonReleased(MouseButtonReleasedEvent& event) noexcept
{
	MouseStates[event.GetMouseButtonCode()] = false;
	MouseEventBuffer.emplace(MakeUnique<MouseButtonReleasedEvent>(static_cast<MouseButtonCode>(event.GetMouseButtonCode())));
	PreventBufferOverflow(MouseEventBuffer);
	return true;
}

bool InputManager::OnMouseMoved(MouseMovedEvent& event) noexcept
{
	MouseEventBuffer.emplace(MakeUnique<MouseMovedEvent>(event.GetXPos(), event.GetYPos()));
	PreventBufferOverflow(MouseEventBuffer);
	return true;
}

bool InputManager::OnMouseScrolled(MouseScrolledEvent& event) noexcept
{
	DeltaCarry += event.GetDelta();

	while (std::abs(DeltaCarry) >= WHEEL_DELTA)
	{
		if (DeltaCarry > 0)
			DeltaCarry -= WHEEL_DELTA;
		else
			DeltaCarry += WHEEL_DELTA;
	}
	MouseEventBuffer.emplace(MakeUnique<MouseScrolledEvent>(event.GetXOffset(), event.GetYOffset(), event.GetDelta()));
	PreventBufferOverflow(MouseEventBuffer);

	return true;
}

bool InputManager::OnMouseEnter(MouseEnterEvent& event) noexcept
{
	MouseInWindow = true;
	MouseEventBuffer.emplace(MakeUnique<MouseEnterEvent>());
	PreventBufferOverflow(MouseEventBuffer);
	return true;
}

bool InputManager::OnMouseLeave(MouseLeaveEvent& event) noexcept
{
	MouseInWindow = false;
	MouseEventBuffer.emplace(MakeUnique<MouseLeaveEvent>());
	PreventBufferOverflow(MouseEventBuffer);
	return true;
}

template void InputManager::PreventBufferOverflow<KeyEvent>(std::queue<KeyEvent>&) noexcept;
template void InputManager::PreventBufferOverflow<uint8>(std::queue<uint8>&) noexcept;

template<typename T>
void InputManager::PreventBufferOverflow(std::queue<T>& buffer) noexcept
{
	while (buffer.size() > BufferSize)
	{
		buffer.pop();
	}
}