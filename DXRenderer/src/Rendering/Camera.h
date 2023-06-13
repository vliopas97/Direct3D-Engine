#pragma once

#include "Core\Core.h"

#include <DirectXMath.h>

class Camera
{
public:
	Camera(float fov, float aspectRatio, float nearZ, float farZ);

	void SetProjection(float fov, float aspectRatio, float nearZ, float farZ);

	void SetPosition(const DirectX::XMVECTOR& position);

	void SetRotation(const DirectX::XMVECTOR& rotation);

	inline const DirectX::XMMATRIX& GetProjection() const { return Projection; }
	inline const DirectX::XMMATRIX& GetView() const { return View; }
	inline const DirectX::XMMATRIX& GetViewProjection() const { return ViewProjection; }

	void GUI();

private:
	void UpdateViewMatrix();

private:
	DirectX::XMMATRIX Projection;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX ViewProjection;

	DirectX::XMVECTOR Position = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR Rotation = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
};