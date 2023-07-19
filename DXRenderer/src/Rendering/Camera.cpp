#include "Camera.h"

#include "Core/Application.h"

#include <numbers>
#include <imgui.h>
#include <glm/glm.hpp>

template<typename T>
T wrap_angle(T theta) noexcept
{
	constexpr T twoPi = static_cast<T>(2.0 * std::numbers::pi);
	const T mod = std::fmod(theta, twoPi);
	if (mod > std::numbers::pi)
	{
		return mod - twoPi;
	}
	else if (mod < -std::numbers::pi)
	{
		return mod + twoPi;
	}
	return mod;
}

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

inline void Camera::SetPosition(const DirectX::XMFLOAT3& position)
{
	Position = position;
	UpdateViewMatrix();
}

inline void Camera::SetRotation(const DirectX::XMFLOAT3& rotation)
{
	Rotation = rotation;
	UpdateViewMatrix();
}

void Camera::GUI()
{
	ImGui::Text("Position");
	ImGui::SliderFloat("X", &Position.x, -100.0f, 100.0f);
	ImGui::SliderFloat("Y", &Position.y, -100.0f, 100.0f);
	ImGui::SliderFloat("Z", &Position.z, -100.0f, 100.0f);
	ImGui::Text("Orientation");
	//ImGui::SliderAngle("Roll", &Rotation.z, -180.0f, 180.0f);
	ImGui::SliderAngle("Pitch", &Rotation.x, -180.0f * 0.995f, 180.0f * 0.995f);
	ImGui::SliderAngle("Yaw", &Rotation.y, -180.0f, 180.0f);

	UpdateViewMatrix();
}

void Camera::Tick(float delta)
{
	using namespace DirectX;
	auto& input = Application::GetApp().GetWindow()->Input;
	DirectX::XMFLOAT3 cameraPosition{};
	if (input.IsKeyPressed(0x57))
	{
		cameraPosition = { 0.0f, 0.0f, delta };
		XMStoreFloat3(&cameraPosition, XMVector3Transform(
			XMLoadFloat3(&cameraPosition),
			XMMatrixScaling(TranslationSpeed, TranslationSpeed, TranslationSpeed) *
			XMMatrixRotationRollPitchYaw(Rotation.x, Rotation.y, 0.0f)));
	}
	else if (input.IsKeyPressed(0x53))
	{
		cameraPosition = { 0.0f, 0.0f, -delta };
		XMStoreFloat3(&cameraPosition, XMVector3Transform(
			XMLoadFloat3(&cameraPosition),
			XMMatrixScaling(TranslationSpeed, TranslationSpeed, TranslationSpeed) *
			XMMatrixRotationRollPitchYaw(Rotation.x, Rotation.y, 0.0f)));
	}

	SetPosition({Position.x + cameraPosition.x, Position.y + cameraPosition.y, Position.z + cameraPosition.z});
	cameraPosition = {};

	if (input.IsKeyPressed(0x41))
	{
		cameraPosition = { -delta, 0.0f, 0.0f };
		XMStoreFloat3(&cameraPosition, XMVector3Transform(
			XMLoadFloat3(&cameraPosition),
			XMMatrixScaling(TranslationSpeed, TranslationSpeed, TranslationSpeed) *
			XMMatrixRotationRollPitchYaw(Rotation.x, Rotation.y, 0.0f)));
	}
	else if (input.IsKeyPressed(0x44))
	{
		cameraPosition = { delta, 0.0f, 0.0f };
		XMStoreFloat3(&cameraPosition, XMVector3Transform(
			XMLoadFloat3(&cameraPosition),
			XMMatrixScaling(TranslationSpeed, TranslationSpeed, TranslationSpeed) *
			XMMatrixRotationRollPitchYaw(Rotation.x, Rotation.y, 0.0f)));
	}

	SetPosition({ Position.x + cameraPosition.x, Position.y + cameraPosition.y, Position.z + cameraPosition.z });

	if (Application::GetApp().GetWindow()->IsCursorVisible())
		return;

	float pitch = (Rotation.x);
	float yaw =   (Rotation.y);
	float roll =  (Rotation.z);

	while (auto coords = input.FetchRawInputCoords())
	{
		auto [x, y] = coords.value();
		yaw = wrap_angle(yaw + XMConvertToRadians(RotationSpeed * x));
		pitch = std::clamp(pitch + XMConvertToRadians(RotationSpeed * y),
						   -(float)(0.995f * std::numbers::pi) / 2.0f,
						   (float)(0.995f * std::numbers::pi) / 2.0f);
	}

	SetRotation({ pitch, yaw, roll });
}

inline void Camera::UpdateViewMatrix()
{
	using namespace DirectX;

	DirectX::XMMATRIX transform = XMMatrixRotationRollPitchYaw(-Rotation.x, -Rotation.y, Rotation.z)
		* XMMatrixTranslation(-Position.x, -Position.y, Position.z);
	View = XMMatrixInverse(nullptr, transform);
	ViewProjection = View * Projection;
}