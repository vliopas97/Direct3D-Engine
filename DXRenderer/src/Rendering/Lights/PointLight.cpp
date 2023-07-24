#include "PointLight.h"

#include "Rendering\Graphics.h"

#include <imgui.h>

PointLight::PointLight()
	: Mesh(), Properties(),
	Model(UniformPS<LightProperties>(std::string(typeid(this).name()) + "properties", Properties))
{
}

void PointLight::GUI()
{
	ImGui::Begin("Light Position");
	auto& x = Position.x;
	auto& y = Position.y;
	auto& z = Position.z;

	ImGui::Text("Position");
	ImGui::SliderFloat("X", &x, -20.0f, 20.0f, "%.1f");
	ImGui::SliderFloat("Y", &y, -20.0f, 20.0f, "%.1f");
	ImGui::SliderFloat("Z", &z, -20.0f, 20.0f, "%.1f");
	ImGui::End();
}

void PointLight::Bind()
{
	Model.Bind();
}

void PointLight::Draw()
{
	Mesh.Draw();
}

void PointLight::Update()
{
	Mesh.Transform.Translation = Position;
	Mesh.Update();
}
