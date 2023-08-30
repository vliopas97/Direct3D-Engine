#include "Graphics.h"
#include "Core/Exception.h"

#include <d3dcompiler.h>
#include <source_location>

#include "Buffer.h"
#include "Shader.h"

#include "Actors\Actor.h"

Graphics::Graphics(HWND windowHandle, uint32_t width, uint32_t height)
	:GraphicsCamera{nullptr}, Width(width), Height(height)
{
	CurrentGraphicsContext::GraphicsInfo = this;

	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = Width;
	sd.BufferDesc.Height = Height;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = windowHandle;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	UINT swapCreateFlags = 0u;
#ifndef NDEBUG
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	GRAPHICS_ASSERT(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		swapCreateFlags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&SwapChain,
		&Device,
		nullptr,
		&Context
	));

	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer = nullptr;
	GRAPHICS_ASSERT(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer));

	RTarget = MakeShared<RenderTargetOutput>(backBuffer.Get());

	D3D11_VIEWPORT viewport;
	viewport.Width = (float)Width;
	viewport.Height = (float)Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	Context->RSSetViewports(1u, &viewport);
}

void Graphics::Tick(float delta)
{
	GraphicsCamera->Tick(delta);
	View = GraphicsCamera->GetView();
	Projection = GraphicsCamera->GetProjection();
	ViewProjection = GraphicsCamera->GetViewProjection();

#ifndef NDEBUG
	InfoManager.Reset();
#endif // !NDEBUG

	HRESULT result;
	if (FAILED(result = SwapChain->Present(1u, 0u)))
	{
		if (result == DXGI_ERROR_DEVICE_REMOVED)
		{
			throw GRAPHICS_DEVICE_REMOVED_EXCEPTION(Device->GetDeviceRemovedReason());
		}
		else
		{
			GRAPHICS_EXCEPTION(result);
		}
	}
	ClearColor();
}

void Graphics::ClearColor() noexcept
{
	RTarget->Clear(backgroundColor);
}

const Microsoft::WRL::ComPtr<ID3D11Device>& Graphics::GetDevice() const
{
	return Device;
}

void Graphics::SetCamera(Camera& camera)
{
	GraphicsCamera = &camera;
}

const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& Graphics::GetContext() const
{
	return Context;
}