#include "PointLight.h"

#include "Rendering\Graphics.h"
#include "Rendering/Actors/CameraViewer.h"

#include <imgui.h>

PointLight::PointLight()
	: Mesh(), Properties(),
	Model(UniformPS<LightProperties>(std::string(typeid(this).name()) + "properties", Properties)),
	ShadowMatrix(UniformVS<DirectX::XMMATRIX>(std::string(typeid(this).name()) + "shadowMat", Cam.GetViewProjection(), 3))
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

	auto& p = Rotation.x;
	auto& yaw = Rotation.y;

	ImGui::Text("Rotation");
	ImGui::SliderAngle("Pitch", &p, -180.0f + 0.1f, 180.0f - 0.1f, "%.1f");
	ImGui::SliderAngle("Yaw", &yaw, -180.0f + 0.1f, 180.0f - 0.1f, "%.1f");

	Cam.Projection.ShowData();

	ImGui::End();
}

void PointLight::Bind()
{
	Model.Bind();
	ShadowMatrix.Bind();
}

void PointLight::Submit(size_t channelsIn)
{
	Mesh.Submit(channelsIn);
	Cam.Submit(channelsIn);
}

void PointLight::LinkTechniques()
{
	Mesh.LinkTechniques();
	Cam.LinkTechniques();
}

void PointLight::Tick(float delta)
{
	//DirectX::XMMATRIX transform = DirectX::XMMatrixRotationRollPitchYaw(-Rotation.x, -Rotation.y, Rotation.z)
	//	* DirectX::XMMatrixTranslation(-Position.x, -Position.y, Position.z);
	//ViewProjection = XMMatrixInverse(nullptr, transform) * DirectX::XMMatrixPerspectiveFovLH(5.0f, 16.0f / 9.0f, 0.1f, 400.0f);

	Cam.SetPosition(Position);
	Cam.SetRotation(Rotation);
	Cam.Projection.Tick(delta);

	Mesh.Transform.Translation = Position;
	Mesh.Tick(delta);
}
