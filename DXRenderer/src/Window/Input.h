#pragma once
#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Events/WindowEvent.h"
#include "Events/MouseEvent.h"

#include <queue>
#include <bitset>

class InputManager
{
public:
	InputManager() = default;
	InputManager(const InputManager&) = delete;
	InputManager& operator=(const InputManager&) = delete;

	bool IsKeyPressed(uint8 keycode) const noexcept;
	bool IsKeyBufferEmpty() const noexcept;
	bool IsMouseBufferEmpty() const noexcept;
	void FlushKeyEventBuffer() noexcept;
	std::optional<UniquePtr<Event>> FetchKeyEvent() noexcept;
	std::optional<UniquePtr<Event>> FetchMouseEvent() noexcept;

	bool IsCharBufferEmpty() noexcept;
	void FlushCharBuffer() noexcept;
	uint8 FetchKeyTyped() noexcept;

	void FlushAll() noexcept;

	void EnableAutorepeat() noexcept;
	void DisableAutorepeat() noexcept;
	bool IsAutoRepeatEnabled() const noexcept;

	bool IsMouseInWindow() const noexcept;

	void OnEvent(Event& event);
private:
	bool OnKeyPressed(KeyPressedEvent& event) noexcept;
	bool OnKeyReleased(KeyReleasedEvent& event) noexcept;
	bool OnKeyTyped(KeyTypedEvent& event) noexcept;

	bool OnWindowLostFocus(WindowLostFocusEvent& event) noexcept;
	bool OnWindowClose(WindowCloseEvent& event) noexcept;

	bool OnMouseButtonPressed(MouseButtonPressedEvent& event) noexcept;
	bool OnMouseButtonReleased(MouseButtonReleasedEvent& event) noexcept;
	bool OnMouseMoved(MouseMovedEvent& event) noexcept;
	bool OnMouseScrolled(MouseScrolledEvent& event) noexcept;
	bool OnMouseEnter(MouseEnterEvent& event) noexcept;
	bool OnMouseLeave(MouseLeaveEvent& event) noexcept;

	template<typename T>
	static void PreventBufferOverflow(std::queue<T>& buffer) noexcept;

public:
	bool MouseInWindow = false;

private:
	static constexpr uint32_t NumberOfKeys = 256u;
	static constexpr uint32_t BufferSize = 16u;

	std::bitset<NumberOfKeys> KeyStates;
	std::bitset<8> MouseStates;

	std::queue<UniquePtr<Event>> KeyEventBuffer;
	std::queue<UniquePtr<Event>> MouseEventBuffer;
	std::queue<uint8> CharBuffer;

	bool RepeatEnabled = true;
public:
	int DeltaCarry = 0;
};