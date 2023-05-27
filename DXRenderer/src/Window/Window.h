#pragma once

#include "WindowClass.h"
#include "Input.h"
#include "Rendering/Graphics.h"

#define DEFINE_WINDOW_CLASS(ClassName) friend class ClassName

class Window
{
	using EventCallbackFn = std::function<void(Event&)>;
public:
	Window(uint32_t width = 1920, uint32_t height = 1080, const std::string& name = "Direct3D Window");
	Window(const Window&) = delete;
	~Window();

	void Show(bool show) const;

	void SetWindowName(const std::string& name);

	WinMessage GetWinMessage() const;
	static std::optional<int> ProcessMessages();

	inline uint32_t GetWidth() const { return Width; }
	inline uint32_t GetHeight() const { return Height; }
	Graphics& GetGraphicsContext() const;

	void SetTitle(const std::string& name);

	void OnEvent(Event& event);

private:
	void SetEventCallbackFunction(const EventCallbackFn& fn);

	static LRESULT CALLBACK InitializeWindow(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK WindProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WindProcImpl(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

	bool OnWindowLostFocus(WindowLostFocusEvent& event) noexcept;
	bool OnWindowClose(WindowCloseEvent& event) noexcept;
public:
	InputManager Input;
	std::string Name;
private:
	HWND Handle;
	EventCallbackFn EventCallback;

	RECT Rect;
	uint32_t Width, Height;
	UniquePtr<Graphics> GraphicsContext;
	
	DEFINE_WINDOW_CLASS(WindowClass);
};