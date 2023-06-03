#pragma once

#include "Core/Core.h"
#include "Core/Exception.h"

#include <d3d11.h>
#include <functional>
#include <wrl.h>

class Graphics
{
public:
	template<typename T>
	inline static std::function<void(T*)> Deleter = [](T* ptr)
	{
		ptr->Release();
	};

	Graphics(HWND windowHandle);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics() = default;

	void EndTick();
	void DrawTriangle();
	
	const UniquePtrCustomDeleter<ID3D11DeviceContext>& GetContext() const;
	const UniquePtrCustomDeleter<ID3D11Device>& GetDevice() const;

private:
	void ClearColor(float red, float green, float blue) noexcept;
	void ClearDepth() noexcept;
	
	void SwapBuffers();
	
private:
	UniquePtrCustomDeleter<ID3D11Device>        Device;
	UniquePtrCustomDeleter<IDXGISwapChain>      SwapChain;
	UniquePtrCustomDeleter<ID3D11DeviceContext> Context;
	UniquePtrCustomDeleter<ID3D11RenderTargetView> RenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilView;

	#ifndef NDEBUG
	DXGIInfoManager InfoManager;
	#endif // !NDEBUG

};