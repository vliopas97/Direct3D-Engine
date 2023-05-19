#include <stdint.h>
#include <d3d11.h>
#include "Window/Window.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	Window win;

	WinMessage msg;
	while ((msg = Window::GetAppMessage()).Result > 0)
	{
		TranslateMessage(&msg.Message);
		DispatchMessage(&msg.Message);
	}

	if (msg.Result == -1)
		return -1;
	else
		return msg.Message.wParam;
	return 0;
}