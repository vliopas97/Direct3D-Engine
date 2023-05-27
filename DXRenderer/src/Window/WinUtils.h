#pragma once

#include "Core/Core.h"

struct WindowClassData
{
	WindowClassData(HINSTANCE hInstance);

	operator WNDCLASSEX() const;

	UINT      CbSize = sizeof(WNDCLASSEX);
	UINT      Style = CS_OWNDC;
	WNDPROC   LpfnWndProc = DefWindowProc;
	int       CbClsExtra = 0;
	int       CbWndExtra = 0;
	HINSTANCE HInstance;
	HICON     HIcon = nullptr;
	HCURSOR   HCursor = nullptr;
	HBRUSH    HbrBackground = nullptr;
	LPCWSTR   LpszMenuName = nullptr;
	LPCWSTR   LpszClassName = L"Direct3D Engine";
	HICON     HIconSm = nullptr;
};

struct WinMessage
{
	WinMessage() = default;
	WinMessage(BOOL result, MSG msg)
		: Result(result), Message(msg)
	{
	}

	BOOL Result;
	MSG Message;

	operator int() const
	{
		return Result;
	}
};