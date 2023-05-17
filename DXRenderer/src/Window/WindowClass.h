#pragma once
#include "WinUtils.h"
#include <string>

class WindowClass
{
public:
	static const std::wstring& GetName();
	static HINSTANCE GetInstance();
	static WindowClass& GetWindowClassInstance();
private:
	WindowClass();
	WindowClass(const WindowClass&) = delete;
	WindowClass& operator=(const WindowClass&) = delete;
	~WindowClass();

	WindowClassData ClassData;
	static std::wstring Name;
};

extern WindowClass& (*WindowClassInstance)();