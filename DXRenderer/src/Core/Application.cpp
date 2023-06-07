#include "Application.h"
#include "Rendering\Actors\Cube.h"

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
	auto cursor = LoadCursor(nullptr, IDC_ARROW);
	SetCursor(cursor);

	// TEST
	TransformationIntrinsics trInt;
	trInt.Pitch = trInt.Roll = 40.0f;
	trInt.Z = 4;
	Cubes.emplace_back(MakeUnique<Cube>(trInt));

	TransformationIntrinsics trInt2;
	trInt2.Yaw = 45.0f;
	trInt2.X = 2.0f;
	trInt2.Z = 6.0f;
	Cubes.emplace_back(MakeUnique<Cube>(trInt2));
}

void Application::Tick()
{
	for (auto& c : Cubes)
	{
		c->Pitch += 0.1f;
		c->Update();
		c->Draw();
	}
	MainWindow->GetGraphicsContext().DrawScene();
	MainWindow->GetGraphicsContext().EndTick();
}
