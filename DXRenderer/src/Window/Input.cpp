#include "Input.h"
#include <typeinfo>

#define BIND_EVENT_FN(x) std::bind(&Input::x, this, std::placeholders::_1)

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
		else
			return nullptr;
	}
}

bool Input::IsKeyPressed(uint8 keycode) const noexcept
{
	return KeyStates[keycode];
}

bool Input::IsKeyBufferEmpty() const noexcept
{
	return KeyEventBuffer.empty();
}

void Input::FlushKeyEventBuffer() noexcept
{
	KeyEventBuffer = std::queue<UniquePtr<Event>>();
}

std::optional<UniquePtr<Event>> Input::FetchKeyEvent() noexcept
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

bool Input::IsCharBufferEmpty() noexcept
{
	return CharBuffer.empty();
}

void Input::FlushCharBuffer() noexcept
{
	CharBuffer = std::queue<uint8>();
}

uint8 Input::FetchKeyTyped() noexcept
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

void Input::FlushAll() noexcept
{
	FlushKeyEventBuffer();
	FlushCharBuffer();
}

void Input::EnableAutorepeat() noexcept
{
	RepeatEnabled = true;
}

void Input::DisableAutorepeat() noexcept
{
	RepeatEnabled = false;
}

bool Input::IsAutoRepeatEnabled() const noexcept
{
	return RepeatEnabled;
}

void Input::OnEvent(Event& event)
{
	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(OnKeyPressed));
	dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN(OnKeyReleased));
	dispatcher.Dispatch<KeyTypedEvent>(BIND_EVENT_FN(OnKeyTyped));
}

bool Input::OnKeyPressed(KeyPressedEvent& event) noexcept
{
	KeyStates[event.GetKeycode()] = true;
	KeyEventBuffer.emplace(MakeUnique<KeyPressedEvent>(event.GetKeycode()));
	PreventBufferOverflow(KeyEventBuffer);
	return true;
}

bool Input::OnKeyReleased(KeyReleasedEvent& event) noexcept
{
	KeyStates[event.GetKeycode()] = false;
	KeyEventBuffer.emplace(MakeUnique<KeyReleasedEvent>(event.GetKeycode()));
	PreventBufferOverflow(KeyEventBuffer);
	return true;
}

bool Input::OnKeyTyped(KeyTypedEvent& event) noexcept
{
	CharBuffer.push(event.GetKeycode());
	PreventBufferOverflow(KeyEventBuffer);
	return true;
}

void Input::ClearState() noexcept
{
	KeyStates.reset();
}

template void Input::PreventBufferOverflow<KeyEvent>(std::queue<KeyEvent>&) noexcept;
template void Input::PreventBufferOverflow<uint8>(std::queue<uint8>&) noexcept;

template<typename T>
void Input::PreventBufferOverflow(std::queue<T>& buffer) noexcept
{
	while (buffer.size() > BufferSize)
	{
		buffer.pop();
	}
}