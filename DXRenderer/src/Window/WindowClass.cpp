#include "WindowClass.h"
#include "Window.h"

std::wstring WindowClass::Name = L"Direct3D Engine";
WindowClass& (*WindowClassInstance)() = WindowClass::GetWindowClassInstance;

const std::wstring& WindowClass::GetName()
{
	return Name;
}

HINSTANCE WindowClass::GetInstance()
{
	return WindowClassInstance().ClassData.HInstance;
}

WindowClass& WindowClass::GetWindowClassInstance()
{
	static WindowClass windowClassInstance;
	return windowClassInstance;
}

WindowClass::WindowClass()
	: ClassData(GetModuleHandle(nullptr))
{
	ClassData.LpfnWndProc = Window::InitializeWindow;
	RegisterClassEx((WNDCLASSEX*)&ClassData);

	Name = ClassData.LpszClassName;
}

WindowClass::~WindowClass()
{
	UnregisterClass(Name.c_str(), ClassData.HInstance);
}