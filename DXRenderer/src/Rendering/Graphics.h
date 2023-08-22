#pragma once

#include "Camera.h"
#include "Core/Core.h"
#include "Core/Exception.h"
#include "RenderTarget.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <functional>
#include <wrl.h>

class Graphics
{
public:
	Graphics(HWND windowHandle, uint32_t width, uint32_t height);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics() = default;

	void Tick(float delta);
	
	const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& GetContext() const;
	const Microsoft::WRL::ComPtr<ID3D11Device>& GetDevice() const;

	void SetCamera(Camera& camera);
	inline uint32_t GetWidth() const { return Width; }
	inline uint32_t GetHeight() const { return Height; }
	inline SharedPtr<RenderTarget> GetTarget() { return RTarget; }

	inline const DirectX::XMMATRIX& GetView() { return View; }
	inline const DirectX::XMMATRIX& GetProjection() { return Projection; }
	inline const DirectX::XMMATRIX& GetViewProjection() { return ViewProjection; }

private:
	void ClearColor() noexcept;
	
private:
	Microsoft::WRL::ComPtr<ID3D11Device>        Device;
	Microsoft::WRL::ComPtr<IDXGISwapChain>      SwapChain;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> Context;
	SharedPtr<RenderTarget> RTarget;

	#ifndef NDEBUG
	DXGIInfoManager InfoManager;
	#endif

	Camera* GraphicsCamera;

	uint32_t Width, Height;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Projection;
	DirectX::XMMATRIX ViewProjection;
};