#pragma once

#include "Camera.h"
#include "Core/Core.h"
#include "Core/Exception.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <functional>
#include <wrl.h>

class Graphics
{
public:
	Graphics(HWND windowHandle);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics() = default;

	void Tick(float delta);
	void DrawScene();
	
	const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& GetContext() const;
	const Microsoft::WRL::ComPtr<ID3D11Device>& GetDevice() const;

	Camera& GetCamera();
private:
	void ClearColor(float red, float green, float blue) noexcept;
	void ClearDepth() noexcept;
	
	void SwapBuffers();
	
private:
	Microsoft::WRL::ComPtr<ID3D11Device>        Device;
	Microsoft::WRL::ComPtr<IDXGISwapChain>      SwapChain;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> Context;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilView;

	#ifndef NDEBUG
	DXGIInfoManager InfoManager;
	#endif // !NDEBUG

	Camera GraphicsCamera;
};