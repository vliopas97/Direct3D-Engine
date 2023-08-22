#include "Application.h"
#include "Layer.h"
#include "Rendering\Actors\Cube.h"
#include "Rendering\Actors\Plane.h"
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
	:MainWindow(MakeUnique<Window>())
{
	ASSERT(!Instance);
	Instance = this;
	auto cursor = LoadCursor(nullptr, IDC_ARROW);

	Camera* camera = new Camera();
	camera->SetPosition({ -13.5f,6.0f,3.5f });
	Camera* camera2 = new Camera();
	camera2->SetPosition({ -13.5f,28.8f,-6.4f });
	camera2->SetRotation({ pi / 180.0f * 13.0f, pi / 180.0f * 61.0f , 0.0f });
	Cameras.AddCamera(UniquePtr<Camera>(camera));
	Cameras.AddCamera(UniquePtr<Camera>(camera2));
	MainWindow->GetGraphicsContext().SetCamera(Cameras.GetCamera());

	SetCursor(cursor);

	MainWindow->SetEventCallbackFunction([this](Event& e)
										 {
											 OnEvent(e);
										 });

	// TEST
	TransformationIntrinsics trInt;
	trInt.Sx = trInt.Sy = trInt.Sz = 0.05f;
	trInt.X = -10.0f;
	trInt.Z = 10.0f;
	trInt.Roll = 180.0f;
	trInt.Yaw = -90.0f;
	//Actors.emplace_back(MakeUnique<Model>("Sponza\\sponza.obj", trInt));
	trInt.X = -13.5f;
	trInt.Y = 6.0f;
	trInt.Z = 8.0f;
	trInt.Sx = trInt.Sy = trInt.Sz = 2.f;
	Actors.emplace_back(MakeUnique<Cube>(trInt));

	ImGui = MakeUnique<ImGuiLayer>();
	ImGui->OnAttach();

	for (auto& c : Actors)
		c->LinkTechniques();

	Light = MakeUnique<PointLight>();
	Light->Properties.Ambient = { 0.05f, 0.05f, 0.05f };
	Light->Properties.Position = { -12.0f, 12.0f, 15.0f };
	Light->LinkTechniques();

}

void Application::Tick()
{
	float delta = Benchmarker.GetAndReset();

	MainWindow->GetGraphicsContext().SetCamera(Cameras.GetCamera());

	ImGui->Begin();
	Cameras.GUI();
	Light->Bind();
	for (auto& c : Actors)
	{
		c->Tick(delta);
		c->Draw();
		c->GUI();
	}

	Light->Tick(delta);
	Light->Draw();
	Light->GUI();
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

	RenderGraph::Execute();
	ImGui->End();

	MainWindow->Tick(delta);
	RenderGraph::Reset();
}
