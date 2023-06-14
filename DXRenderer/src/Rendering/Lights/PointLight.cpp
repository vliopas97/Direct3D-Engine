#include "PointLight.h"

#include "Rendering\Graphics.h"

#include <imgui.h>

PointLight::PointLight()
	: Mesh(), Position(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f)),
	Model(MakeUnique<PSConstantBuffer<DirectX::XMFLOAT3>>(DirectX::XMFLOAT3()), Position)
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
	Mesh.Transform.Translation = DirectX::XMLoadFloat3(&Position);
	Mesh.Update();
}
