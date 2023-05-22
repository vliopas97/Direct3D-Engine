#include <stdint.h>
#include <d3d11.h>
#include "Window/Window.h"
#include <sstream>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	Window win;

	WinMessage msg;
	while ((msg = Window::GetAppMessage()).Result > 0)
	{
		TranslateMessage(&msg.Message);
		DispatchMessage(&msg.Message);
		
		while (!win.Input.IsMouseBufferEmpty())
		{
			auto e = win.Input.FetchMouseEvent();
			if (e.has_value() && e.value())
			{
				if (e->get()->GetEventType() == EventType::MouseMoved)
				{
					UniquePtr<MouseMovedEvent> ptr(reinterpret_cast<MouseMovedEvent*>(e.value().release()));
					std::ostringstream oss;
					oss << "Mouse Pos: (" << ptr.get()->GetXPos() << ", " << ptr.get()->GetYPos() << ")" << std::endl;
					win.SetTitle(oss.str());
				}
			}
		}
	}

	if (msg.Result == -1)
		return -1;
	else
		return msg.Message.wParam;
	return 0;
}