#include "Input.h"
#include <typeinfo>

#define BIND_EVENT_FN(x) std::bind(&InputManager::x, this, std::placeholders::_1)

namespace
{
	UniquePtr<Event> UniquePtrDispatcher(Event* e)
	{
		const auto& typeID = typeid(*e);
		if (typeID == typeid(KeyPressedEvent))
			return std::unique_ptr<KeyPressedEvent>(dynamic_cast<KeyPressedEvent*>(e));
		else if (typeID == typeid(KeyReleasedEvent))
			return std::unique_ptr<KeyReleasedEvent>(dynamic_cast<KeyReleasedEvent*>(e));
		else if (typeID == typeid(KeyTypedEvent))
			return std::unique_ptr<KeyTypedEvent>(dynamic_cast<KeyTypedEvent*>(e));
		else if (typeID == typeid(MouseButtonPressedEvent))
			return std::unique_ptr<MouseButtonPressedEvent>(dynamic_cast<MouseButtonPressedEvent*>(e));
		else if (typeID == typeid(MouseButtonReleasedEvent))
			return std::unique_ptr<MouseButtonReleasedEvent>(dynamic_cast<MouseButtonReleasedEvent*>(e));
		else if (typeID == typeid(MouseMovedEvent))
			return std::unique_ptr<MouseMovedEvent>(dynamic_cast<MouseMovedEvent*>(e));
		else if (typeID == typeid(MouseScrolledEvent))
			return std::unique_ptr<MouseScrolledEvent>(dynamic_cast<MouseScrolledEvent*>(e));
		else
			return nullptr;
	}
}

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
		return std::optional<UniquePtr<Event>>(UniquePtrDispatcher(eventPtr));
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
		return std::optional<UniquePtr<Event>>(UniquePtrDispatcher(eventPtr));
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