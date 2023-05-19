#include "Window.h"
#include <sstream>
#include <iostream>

#define GENERATE_KEYBOARD_EVENT(Event)  {Event event(static_cast<uint8>(wParam));\
										OnEvent(event);}\
										break\

Window::Window(uint32_t width, uint32_t height, const std::string& name)
	: EventHandler()
{
	Handle = CreateWindow(WindowClass::GetName().c_str(),
						  std::wstring(name.begin(), name.end()).c_str(),
						  WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
						  200, 200, width, height,
						  nullptr, nullptr, WindowClass::GetInstance(), this);
	if(!Handle)
		throw WIN_EXCEPTION_LAST_ERROR;

	Show(true);
}

Window::~Window()
{
	DestroyWindow(Handle);
}

void Window::Show(bool show) const
{
	ShowWindow(Handle, show ? SW_SHOW : SW_HIDE);
}

void Window::SetWindowName(const std::string& name)
{
	std::wstring wName(name.begin(), name.end());
	SetWindowText(Handle, wName.c_str());
}

void Window::OnEvent(Event& event)
{
	EventHandler.OnEvent(event);
}


void Window::SetEventCallbackFunction(const EventCallbackFn& fn)
{
	EventCallback = fn;
}

LRESULT Window::InitializeWindow(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CREATE)
	{
		const CREATESTRUCTW* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
		Window* const windowPtr = static_cast<Window*>(createStruct->lpCreateParams);
		SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(windowPtr));
		SetWindowLongPtr(windowHandle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindProc));

		return windowPtr->WindProcImpl(windowHandle, message, wParam, lParam);
	}
	return DefWindowProc(windowHandle, message, wParam, lParam);
}

LRESULT CALLBACK Window::WindProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	Window* const windowPtr = reinterpret_cast<Window*>(GetWindowLongPtr(windowHandle, GWLP_USERDATA));
	return windowPtr->WindProcImpl(windowHandle, message, wParam, lParam);
}

LRESULT Window::WindProcImpl(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CLOSE:
			PostQuitMessage(WM_CLOSE);
			break;
		case WM_KEYDOWN:
			GENERATE_KEYBOARD_EVENT(KeyPressedEvent);
		case WM_CHAR:
			GENERATE_KEYBOARD_EVENT(KeyTypedEvent);
		case WM_KEYUP:
			GENERATE_KEYBOARD_EVENT(KeyReleasedEvent);
		break;
		default:
			break;
	}

	return DefWindowProc(windowHandle, message, wParam, lParam);
}
