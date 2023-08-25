#include "Camera.h"

#include "Core/Application.h"
#include "Actors/CameraViewer.h"

#include <numbers>
#include <imgui.h>
#include <glm/glm.hpp>

uint32_t Camera::UID = 0;

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


Projection::Projection()
	:FovY(5.0f), AspectRatio(16.0f / 9.0f), NearZ(0.1f), FarZ(400.0f)
{
	ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(FovY, AspectRatio, NearZ, FarZ);
}

Projection::Projection(float fovY, float aspectRatio, float nearZ, float farZ)
	: FovY(fovY), AspectRatio(aspectRatio), NearZ(nearZ), FarZ(farZ)
{
	ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(FovY, AspectRatio, NearZ, FarZ);
}

const DirectX::XMMATRIX& Projection::GetMatrix() const
{
	return ProjectionMatrix;
}

void Projection::ShowData()
{
	ImGui::SliderFloat("FovY", &FovY, 0.01f, 8.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
	ImGui::SliderFloat("AspectRatio", &AspectRatio, 0.01f, 4.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
	ImGui::SliderFloat("Near Z", &NearZ, 0.01f, 400.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
	ImGui::SliderFloat("Far Z", &FarZ, 0.01f, 400.0f, "%.2f", ImGuiSliderFlags_Logarithmic);

	ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(FovY, AspectRatio, NearZ, FarZ);
}

void Projection::Tick(float delta)
{
	ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(FovY, AspectRatio, NearZ, FarZ);
}

Camera::Camera()
	:Projection(), View(DirectX::XMMatrixIdentity()),
	CameraObject(MakeUnique<CameraViewer>())
{
	ViewProjection = View * Projection.GetMatrix();
	Tag = std::string("Camera " + std::to_string(UID++));
}

inline void Camera::SetPosition(const DirectX::XMFLOAT3& position)
{
	Position = position;
	CameraObject->SetPosition(Position);
	UpdateViewMatrix();
}

inline void Camera::SetRotation(const DirectX::XMFLOAT3& rotation)
{
	Rotation = rotation;
	CameraObject->SetRotation(Rotation);
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

	ImGui::Text("Projection Values");
	this->Projection.ShowData();

	UpdateViewMatrix();
}

void Camera::Tick(float delta)
{
	using namespace DirectX;

	CameraObject->SetPosition(Position);
	CameraObject->SetRotation(Rotation);
	CameraObject->Tick(delta);

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
	this->Projection.Tick(delta);
}

void Camera::LinkTechniques()
{
	CameraObject->LinkTechniques();
}

void Camera::Draw()
{
	CameraObject->Draw();
}

inline void Camera::UpdateViewMatrix()
{
	using namespace DirectX;

	DirectX::XMMATRIX transform = XMMatrixRotationRollPitchYaw(-Rotation.x, -Rotation.y, Rotation.z)
		* XMMatrixTranslation(-Position.x, -Position.y, Position.z);
	View = XMMatrixInverse(nullptr, transform);
	ViewProjection = View * Projection.GetMatrix();
}

void CameraGroup::AddCamera(UniquePtr<Camera> camera)
{
	Cameras.emplace_back(std::move(camera));
}

Camera& CameraGroup::GetCamera()
{
	return *Cameras[SelectedCameraIndex];
}

void CameraGroup::SetCameraAsSelected()
{
	CurrentGraphicsContext::GraphicsInfo->SetCamera(GetCamera());
}

void CameraGroup::GUI()
{
	if (ImGui::Begin("Cameras"))
	{
		if (ImGui::BeginCombo("Active Camera", Cameras[SelectedCameraIndex]->GetTag().c_str()))
		{
			for (int i = 0; i < Cameras.size(); i++)
			{
				const bool isSelected = i == SelectedCameraIndex;
				if (ImGui::Selectable(Cameras[i]->GetTag().c_str(), isSelected))
				{
					SelectedCameraIndex = i;
				}
			}
			ImGui::EndCombo();
		}

		GetCamera().GUI();
	}
	ImGui::End();
}

void CameraGroup::LinkTechniques()
{
	for (auto& camera : Cameras)
		camera->LinkTechniques();
}

void CameraGroup::Draw()
{
	for (size_t i = 0; i < Cameras.size(); i++)
	{
		if(i != SelectedCameraIndex)
			Cameras[i]->Draw();
	}
}
