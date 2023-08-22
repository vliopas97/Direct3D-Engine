#include "Application.h"
#include "Layer.h"
#include "Rendering\CurrentGraphicsContext.h"

#include <imgui.h>
#include <backends\imgui_impl_dx11.cpp>
#include <backends\imgui_impl_win32.h>

void ImGui_ImplWin32_InitPlatformInterface();

void ImGuiLayer::OnAttach()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.WantCaptureKeyboard = true;
	io.WantCaptureMouse = true;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();

	const auto& device = CurrentGraphicsContext::Device();
	const auto& context = CurrentGraphicsContext::Context();
	ImGui_ImplDX11_Init(device.Get(), context.Get());
	ImGui_ImplWin32_Init(Application::GetApp().GetWindow()->GetHandle());
}

void ImGuiLayer::Render()
{
	static bool showDemoWindow = false;
	if (showDemoWindow)
		ImGui::ShowDemoWindow(&showDemoWindow);

	//Application::GetApp().GetWindow()->GetGraphicsContext().GetCamera().GUI();
}

void ImGuiLayer::OnDetach()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiLayer::OnEvent(Event& e)
{
	ImGuiIO& io = ImGui::GetIO();
	e.Handled = e.GetCategory() == EventCategory::MouseEvents && io.WantCaptureMouse;
	e.Handled = e.GetCategory() == EventCategory::KeyEvents && io.WantCaptureKeyboard;
}

void ImGuiLayer::Begin()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiLayer::End()
{
	ImGuiIO& io = ImGui::GetIO();
	Application& app = Application::GetApp();
	io.DisplaySize = ImVec2((float)app.GetWindow()->GetWidth(), (float)app.GetWindow()->GetHeight());

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}


