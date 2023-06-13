#include "Camera.h"
#include <imgui.h>

Camera::Camera(float fov, float aspectRatio, float nearZ, float farZ)
	:Projection(DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearZ, farZ)),
	View(DirectX::XMMatrixIdentity())
{
	ViewProjection = View * Projection;
}

inline void Camera::SetProjection(float fov, float aspectRatio, float nearZ, float farZ)
{
	Projection = DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearZ, farZ);
	ViewProjection = View * Projection;
}

inline void Camera::SetPosition(const DirectX::XMVECTOR& position)
{
	Position = position;
	UpdateViewMatrix();
}

inline void Camera::SetRotation(const DirectX::XMVECTOR& rotation)
{
	Rotation = rotation;
	UpdateViewMatrix();
}

void Camera::GUI()
{
	auto& x = Position.m128_f32[0];
	auto& y = Position.m128_f32[1];
	auto& z = Position.m128_f32[2];

	auto& pitch = Rotation.m128_f32[0];
	auto& yaw = Rotation.m128_f32[1];
	auto& roll = Rotation.m128_f32[2];

	ImGui::Text("Position");
	ImGui::SliderFloat("X", &x, -100.0f, 100.0f);
	ImGui::SliderFloat("Y", &y, -100.0f, 100.0f);
	ImGui::SliderFloat("Z", &z, -100.0f, 100.0f);
	ImGui::Text("Orientation");
	ImGui::SliderAngle("Roll", &roll, -180.0f, 180.0f);
	ImGui::SliderAngle("Pitch", &pitch, -180.0f, 180.0f);
	ImGui::SliderAngle("Yaw", &yaw, -180.0f, 180.0f);

	UpdateViewMatrix();
}

inline void Camera::UpdateViewMatrix()
{
	using namespace DirectX;
	DirectX::XMVECTOR rotation = XMVectorSet(-XMVectorGetX(Rotation),
											 -XMVectorGetY(Rotation),
											 XMVectorGetZ(Rotation),
											 XMVectorGetW(Rotation));
	DirectX::XMVECTOR position = XMVectorSet(-XMVectorGetX(Position),
											 -XMVectorGetY(Position),
											 XMVectorGetZ(Position),
											 XMVectorGetW(Position));
	DirectX::XMMATRIX transform = XMMatrixRotationRollPitchYawFromVector(rotation) * XMMatrixTranslationFromVector(position);
	View = XMMatrixInverse(nullptr, transform);
	ViewProjection = View * Projection;
}