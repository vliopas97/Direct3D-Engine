#pragma once
#include "Events/Event.h"
#include "Window/Window.h"
#include "Timer.h"

#include <vector>

class Application
{
public:
	static Application& GetApp();
	static WinMessage GetAppMessage();

	int Run();

private:
	Application();
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	void Tick();
private:
	UniquePtr<Window> MainWindow;

	Timer Benchmarker;
	
	std::vector <UniquePtr< class Cube >> Cubes;
};