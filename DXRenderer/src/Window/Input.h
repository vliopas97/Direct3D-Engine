#pragma once
#include "Events/Event.h"
#include "Events/KeyEvent.h"

#include <queue>
#include <bitset>

class Input
{
public:
	Input() = default;
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;

	bool IsKeyPressed(uint8 keycode) const noexcept;
	bool IsKeyBufferEmpty() const noexcept;
	void FlushKeyEventBuffer() noexcept;
	std::optional<UniquePtr<Event>> FetchKeyEvent() noexcept;

	bool IsCharBufferEmpty() noexcept;
	void FlushCharBuffer() noexcept;
	uint8 FetchKeyTyped() noexcept;

	void FlushAll() noexcept;

	void EnableAutorepeat() noexcept;
	void DisableAutorepeat() noexcept;
	bool IsAutoRepeatEnabled() const noexcept;

	void OnEvent(Event& event);
	void ClearState() noexcept;
private:
	bool OnKeyPressed(KeyPressedEvent& event) noexcept;
	bool OnKeyReleased(KeyReleasedEvent& event) noexcept;
	bool OnKeyTyped(KeyTypedEvent& event) noexcept;

	template<typename T>
	static void PreventBufferOverflow(std::queue<T>& buffer) noexcept;

private:
	static constexpr uint32_t NumberOfKeys = 256u;
	static constexpr uint32_t BufferSize = 16u;

public:
	std::bitset<NumberOfKeys> KeyStates;
	std::queue<UniquePtr<Event>> KeyEventBuffer;
	std::queue<uint8> CharBuffer;

	bool RepeatEnabled = true;
};