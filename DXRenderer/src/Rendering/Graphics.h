#pragma once

#include "Core/Core.h"
#include "Core/Exception.h"

#include <d3d11.h>
#include <functional>

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

	void SwapBuffers();
	void ClearColor(float red, float green, float blue) noexcept;
	void DrawTriangle();

	const UniquePtrCustomDeleter<ID3D11Device>& GetDevice() const;
	const UniquePtrCustomDeleter<ID3D11DeviceContext>& GetContext() const;
private:
	UniquePtrCustomDeleter<ID3D11Device>        Device;
	UniquePtrCustomDeleter<IDXGISwapChain>      SwapChain;
	UniquePtrCustomDeleter<ID3D11DeviceContext> Context;
	UniquePtrCustomDeleter<ID3D11RenderTargetView> RenderTargetView;

	#ifndef NDEBUG
	DXGIInfoManager InfoManager;
	#endif // !NDEBUG

};