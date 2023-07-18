#include "Application.h"
#include "Layer.h"
#include "Rendering\Actors\Cube.h"
#include "Rendering\Actors\Model.h"

#include "Rendering/ResourcePool.h"

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
	:MainWindow(MakeUnique<Window>()), Light()
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
	trInt.Yaw = 180.0f;
	trInt.Roll = 180.0f;
	trInt.Z = 15.0f;
	trInt.X = 2.0f;
	trInt.Y = -10.0f;
	Cubes.emplace_back(MakeUnique<Model>("Nanosuit\\nanosuit.obj", trInt));

	ImGui = MakeUnique<ImGuiLayer>();
	ImGui->OnAttach();

	Light.Position = { 0.0f, 5.0f, 12.0f };
}

void Application::Tick()
{
	float delta = Benchmarker.GetAndReset();

	ImGui->Begin();
	Light.Bind();
	for (auto& c : Cubes)
	{
		c->Update();
		c->Draw();
		if (auto ptr = dynamic_cast<Model*>(c.get()))
		{
			ptr->GUI();
			continue;
		}
	}
	Light.Update();
	Light.Draw();
	Light.GUI();
	ImGui->Render();

	if (MainWindow->Input.IsKeyPressed(VK_INSERT))
	{
		if (MainWindow->IsCursorVisible())
		{
			MainWindow->HideCursor();
			MainWindow->Input.SetRawInput(true);
		}
		else
		{
			MainWindow->ShowCursor();
			MainWindow->Input.SetRawInput(false);
		}
	}

	ImGui->End();

	MainWindow->Tick(delta);
}
