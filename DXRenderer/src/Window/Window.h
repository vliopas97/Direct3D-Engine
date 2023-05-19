#pragma once

#include "WindowClass.h"
#include "Input.h"

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

	inline WinMessage GetWinMessage() const { return GetMessageImpl(Handle); }
	inline static WinMessage GetAppMessage() { return GetMessageImpl(nullptr); }

	void OnEvent(Event& event);
private:
	inline static WinMessage GetMessageImpl(HWND handle)
	{
		MSG msg;
		auto result = GetMessage(&msg, handle, 0, 0);
		return WinMessage(result, msg);
	}

	void SetEventCallbackFunction(const EventCallbackFn& fn);
	
	static LRESULT CALLBACK InitializeWindow(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK WindProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WindProcImpl(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
	static void SetWindowName(HWND handle,const std::string& name);

	DEFINE_WINDOW_CLASS(WindowClass);

public:
	Input EventHandler;

private:
	HWND Handle;
	EventCallbackFn EventCallback;
};