#pragma once
#include "Events/Event.h"
#include "Window/Window.h"
#include "Rendering\Lights\PointLight.h"
#include "Timer.h"

#include <vector>

class Application
{
public:
	static Application& GetApp();
	static WinMessage GetAppMessage();
	int Run();
	const UniquePtr<Window>& GetWindow();

	void OnEvent(Event& e);

private:
	Application();
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	void Tick();
private:
	uint32_t X = 0, Y = 0;
	UniquePtr<Window> MainWindow;
	UniquePtr<class ImGuiLayer> ImGui;
	Timer Benchmarker;
	
	std::vector <UniquePtr< class Actor >> Cubes;
	PointLight Light;
	static Application* Instance;
};