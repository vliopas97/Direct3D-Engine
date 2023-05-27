#include <stdint.h>
#include <d3d11.h>
#include <sstream>

#include "Core/Exception.h"
#include "Core/Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	int msg;
	EXCEPTION_WRAP(
		auto & application = Application::GetApp();
		msg = application.Run();
		);

	return msg;
}