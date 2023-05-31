#pragma once

#include "Core/Core.h"
#include "Core/Exception.h"

#include <d3d11.h>
#include <functional>

class Graphics
{
	template<typename T>
	using UniquePtr = std::unique_ptr<T, std::function<void(T*)>>;

	template<typename T>
	inline static std::function<void(T*)> Deleter = [](T* ptr)
	{
		ptr->Release();
	};

public:
	Graphics(HWND windowHandle);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics() = default;

	void SwapBuffers();
	void ClearColor(float red, float green, float blue) noexcept;
	void DrawTriangle();
private:
	UniquePtr<ID3D11Device>        Device;
	UniquePtr<IDXGISwapChain>      SwapChain;
	UniquePtr<ID3D11DeviceContext> Context;
	UniquePtr<ID3D11RenderTargetView> RenderTargetView;

	#ifndef NDEBUG
	DXGIInfoManager InfoManager;
	#endif // !NDEBUG

};