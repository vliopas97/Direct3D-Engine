#include <stdint.h>
#include <d3d11.h>
#include "Core/Application.h"
#include <sstream>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	auto& application = Application::GetApp();
	int msg = application.Run();

	return msg;
}