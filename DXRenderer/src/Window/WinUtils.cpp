#include "WinUtils.h"


WindowClassData::WindowClassData(HINSTANCE hInstance)
	: HInstance(hInstance)
{
}

WindowClassData::operator WNDCLASSEX() const
{
	WNDCLASSEX wc{ 0 };
	wc.cbSize = CbSize;
	wc.style = Style;
	wc.lpfnWndProc = LpfnWndProc;
	wc.cbClsExtra = CbClsExtra;
	wc.cbWndExtra = CbWndExtra;
	wc.hInstance = HInstance;
	wc.hIcon = HIcon;
	wc.hCursor = HCursor;
	wc.hbrBackground = HbrBackground;
	wc.lpszMenuName = LpszMenuName;
	wc.lpszClassName = LpszClassName;
	wc.hIconSm = HIconSm;
	return wc;
}