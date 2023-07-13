#pragma once

#include "Core\Core.h"

#include <DirectXMath.h>

class Camera
{
public:
	Camera(float fov, float aspectRatio, float nearZ, float farZ);

	void SetProjection(float fov, float aspectRatio, float nearZ, float farZ);

	void SetPosition(const DirectX::XMFLOAT3& position);
	void SetRotation(const DirectX::XMFLOAT3& rotation);

	inline DirectX::XMFLOAT3 GetPosition() const { return Position; }
	inline DirectX::XMFLOAT3 GetRotation() const { return Rotation; }

	inline const DirectX::XMMATRIX& GetProjection() const { return Projection; }
	inline const DirectX::XMMATRIX& GetView() const { return View; }
	inline const DirectX::XMMATRIX& GetViewProjection() const { return ViewProjection; }

	void GUI();
	void Tick(float delta);
private:
	void UpdateViewMatrix();

private:
	DirectX::XMMATRIX Projection;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX ViewProjection;

	DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 Rotation = { 0.0f, 0.0f, 0.0f };

	float TranslationSpeed = 2.0f, RotationSpeed = 0.05f;

};