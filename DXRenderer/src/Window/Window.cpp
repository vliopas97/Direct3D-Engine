#include "Window.h"
#include <sstream>
#include <iostream>

#define BIND_EVENT_FN(x) std::bind(&Window::x, this, std::placeholders::_1)

#define GEN_KEYBOARD_EVENT(Event)		{Event event(static_cast<uint8>(wParam));\
										OnEvent(event);}\
										break
#define GEN_KEYPRESSED_EVENT()		    {KeyPressedEvent event(static_cast<uint8>(wParam), static_cast<bool>(lParam & 0x40000000));\
										OnEvent(event);}\
										break

#define GEN_KEYRELEASED_EVENT() GEN_KEYBOARD_EVENT(KeyReleasedEvent)
#define GEN_KEYTYPED_EVENT() GEN_KEYBOARD_EVENT(KeyTypedEvent)

#define GEN_WINDOWCLOSE_EVENT()        {WindowCloseEvent event;\
										OnEvent(event);}\
										break
#define GEN_WINDOWLOSTFOCUS_EVENT()     {WindowLostFocusEvent event;\
										OnEvent(event);}\
										break

#define GEN_MOUSEBUTTON_EVENT(Event , MouseButtonCode) {Event event(MouseButtonCode);\
														OnEvent(event); }\
														break

#define GEN_MOUSEBUTTONPRESSED_EVENT(MouseButtonCode) GEN_MOUSEBUTTON_EVENT(MouseButtonPressedEvent, MouseButtonCode)
#define GEN_MOUSEBUTTONRELEASED_EVENT(MouseButtonCode) GEN_MOUSEBUTTON_EVENT(MouseButtonReleasedEvent, MouseButtonCode)

#define GEN_MOUSEDISPLACEMENT_EVENT(Event) { POINTS point = MAKEPOINTS(lParam);\
											Event event(point.x, point.y);\
											OnEvent(event);}\
											break

#define GEN_MOUSEMOVED_EVENT() GEN_MOUSEDISPLACEMENT_EVENT(MouseMovedEvent)
#define GEN_MOUSESCROLLED_EVENT() GEN_MOUSEDISPLACEMENT_EVENT(MouseScrolledEvent)

#define GEN_MOUSEENTER_EVENT() 		{SetCapture(Handle);\
									MouseEnterEvent event;\
									OnEvent(event);}

#define GEN_MOUSELEAVE_EVENT() 		{ReleaseCapture();\
									MouseLeaveEvent event;\
									OnEvent(event);}

Window::Window(uint32_t width, uint32_t height, const std::string& name)
	: Name(name), Width(width), Height(height)
{
	Rect.left = 100;
	Rect.right = Rect.left + width;
	Rect.top = 100;
	Rect.bottom = Rect.top + height;

	if (AdjustWindowRect(&Rect, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE) == 0)
		throw WIN_EXCEPTION_LAST_ERROR;

	Handle = CreateWindow(WindowClass::GetName().c_str(),
						  std::wstring(name.begin(), name.end()).c_str(),
						  WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
						  200, 200, Rect.right - Rect.left, Rect.bottom - Rect.top,
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

void Window::SetTitle(const std::string& name)
{
	Name = name;
	if (SetWindowTextA(Handle, Name.c_str()) == 0)
	{
		throw WIN_EXCEPTION_LAST_ERROR;
	}
}

void Window::OnEvent(Event& event)
{
	EventDispatcher dispatcher(event);

	dispatcher.Dispatch<WindowLostFocusEvent>(BIND_EVENT_FN(OnWindowLostFocus));
	dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

	Input.OnEvent(event);
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
		// Window Events
		case WM_CLOSE:
			GEN_WINDOWCLOSE_EVENT();
		case WM_KILLFOCUS:
			GEN_WINDOWLOSTFOCUS_EVENT();

		// Keyboard Events
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			GEN_KEYPRESSED_EVENT();
		case WM_CHAR:
			GEN_KEYTYPED_EVENT();
		case WM_KEYUP:
		case WM_SYSKEYUP:
			GEN_KEYRELEASED_EVENT();

		// Mouse Events
		case WM_MOUSEMOVE:
			{
				const POINTS points = MAKEPOINTS(lParam);
			
				if ((points.x >= 0 || points.x < Width || points.y >= 0 || points.y < Height))
				{
					MouseMovedEvent event(points.x, points.y);
					OnEvent(event);
					if (Input.IsMouseInWindow())
						GEN_MOUSEENTER_EVENT();
				}
				else
				{
					if (wParam & (MK_LBUTTON | MK_RBUTTON))
					{
						MouseMovedEvent event(points.x, points.y);
						OnEvent(event);
					}
					else
						GEN_MOUSELEAVE_EVENT();
				}
			}
			break;
		case WM_LBUTTONDOWN:
			GEN_MOUSEBUTTONPRESSED_EVENT(MouseButtonCode::ButtonLeft);
		case WM_RBUTTONDOWN:
			GEN_MOUSEBUTTONPRESSED_EVENT(MouseButtonCode::ButtonRight);
		case WM_LBUTTONUP:
			{
				const POINTS points = MAKEPOINTS(lParam);
				MouseButtonReleasedEvent event(MouseButtonCode::ButtonLeft);
				if (!(points.x >= 0 || points.x < Width || points.y >= 0 || points.y < Height))
					GEN_MOUSELEAVE_EVENT();
			}
			break;
		case WM_RBUTTONUP:
			{
				const POINTS points = MAKEPOINTS(lParam);
				MouseButtonReleasedEvent event(MouseButtonCode::ButtonRight);
				if (!(points.x >= 0 || points.x < Width || points.y >= 0 || points.y < Height))
					GEN_MOUSELEAVE_EVENT();
			}
			break;
		case WM_MOUSEWHEEL:
			{
				POINTS point = MAKEPOINTS(lParam); 
				MouseScrolledEvent event(point.x, point.y, GET_WHEEL_DELTA_WPARAM(wParam)); 
				OnEvent(event);
			}
			break;
		default:
			break;
	}

	return DefWindowProc(windowHandle, message, wParam, lParam);
}

bool Window::OnWindowLostFocus(WindowLostFocusEvent& event) noexcept
{
	Input.ResetKeyStates();
	return true;
}

bool Window::OnWindowClose(WindowCloseEvent& event) noexcept
{
	PostQuitMessage(WM_CLOSE);
	return true;
}