#include "Application.h"
#include "Layer.h"
#include "Rendering\Actors\Cube.h"

Application* Application::Instance = nullptr;

Application& Application::GetApp()
{
	//static Application application;
	if (!Instance)
		Instance = new Application();
	return *Instance;
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

const UniquePtr<Window>& Application::GetWindow()
{
	return MainWindow;
}

void Application::OnEvent(Event& e)
{
	EventDispatcher dispathcer(e);

	ImGui->OnEvent(e);
	MainWindow->OnEvent(e);
}

Application::Application()
	:MainWindow(MakeUnique<Window>())
{
	ASSERT(!Instance);
	Instance = this;
	auto cursor = LoadCursor(nullptr, IDC_ARROW);
	SetCursor(cursor);

	MainWindow->SetEventCallbackFunction([this](Event& e)
										 {
											 OnEvent(e);
										 });

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

	ImGui = MakeUnique<ImGuiLayer>();
	ImGui->OnAttach();
}

void Application::Tick()
{
	ImGui->Begin();
	for (auto& c : Cubes)
	{
		c->Pitch += 0.1f;
		c->Update();
		c->Draw();
	}
	ImGui->Render();
	ImGui->End();
	MainWindow->GetGraphicsContext().EndTick();
}
