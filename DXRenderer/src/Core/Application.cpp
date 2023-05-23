#include "Application.h"

Application& Application::GetApp()
{
	static Application application;
	return application;
}

WinMessage Application::GetAppMessage()
{
	MSG msg;
	auto result = GetMessage(&msg, nullptr, 0, 0);
	return WinMessage(result, msg);
}

int Application::Run()
{
	while (true)
	{
		if( const auto ecode = Window::ProcessMessages() )
			return *ecode;
		Tick();
	}
}

Application::Application()
	:MainWindow(MakeUnique<Window>())
{
}

void Application::Tick()
{
}
