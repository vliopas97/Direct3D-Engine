#pragma once

#include "Core\Core.h"

#include <DirectXMath.h>
#include <string>

class Projection
{
public:
	Projection();
	Projection(float fovY, float aspectRatio, float nearZ, float farZ);

	const DirectX::XMMATRIX& GetMatrix() const;

	void ShowData();
	void Tick(float delta);
private:
	float FovY;
	float AspectRatio;
	float NearZ;
	float FarZ;
	DirectX::XMMATRIX ProjectionMatrix;
};

class Camera
{
public:
	Camera();

	void SetPosition(const DirectX::XMFLOAT3& position);
	void SetRotation(const DirectX::XMFLOAT3& rotation);

	inline DirectX::XMFLOAT3 GetPosition() const { return Position; }
	inline DirectX::XMFLOAT3 GetRotation() const { return Rotation; }

	inline const DirectX::XMMATRIX& GetProjection() const { return Projection.GetMatrix(); }
	inline const DirectX::XMMATRIX& GetView() const { return View; }
	inline const DirectX::XMMATRIX& GetViewProjection() const { return ViewProjection; }

	inline const std::string& GetTag() const { return Tag; }

	void GUI();
	void Tick(float delta);

	void LinkTechniques();
	void Draw();

private:
	void UpdateViewMatrix();

private:
	Projection Projection;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX ViewProjection;

	DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 Rotation = { 0.0f, 0.0f, 0.0f };

	float TranslationSpeed = 10.0f, RotationSpeed = 0.05f;
	std::string Tag;

	UniquePtr<class CameraViewer> CameraObject;

	static uint32_t UID;
};

class CameraGroup
{
public:
	void AddCamera(UniquePtr<Camera> camera);

	Camera& GetCamera();
	void SetCameraAsSelected();
	void GUI();

	void LinkTechniques();
	void Draw();

private:
	std::vector<UniquePtr<Camera>> Cameras;
	uint32_t SelectedCameraIndex = 0;
};