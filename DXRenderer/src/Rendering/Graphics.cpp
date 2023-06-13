#include "Graphics.h"
#include "Core/Exception.h"

#include <d3dcompiler.h>
#include <source_location>

#include "Buffer.h"
#include "Shader.h"

#include "Actors\Actor.h"

Graphics::Graphics(HWND windowHandle)
	:GraphicsCamera(5.0f, 16.0f / 9.0f, 0.1f, 10.0f)
{
	CurrentGraphicsContext::GraphicsInfo = this;

	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
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

	Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer = nullptr;
	GRAPHICS_ASSERT(SwapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backBuffer));
	GRAPHICS_ASSERT(Device->CreateRenderTargetView(backBuffer.Get(), nullptr, &RenderTargetView));

	// Create Depth Buffer
	DepthBuffer depthBuffer(DepthStencilView);
	depthBuffer.Bind();

	Context->OMSetRenderTargets(1, RenderTargetView.GetAddressOf(), DepthStencilView.Get());

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = 1920.0f;
	viewport.Height = 1080.0f;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	Context->RSSetViewports(1, &viewport);
}

void Graphics::SwapBuffers()
{
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
}

void Graphics::EndTick()
{
	SwapBuffers();
	ClearColor(1.0f, 0.5f, 0.0f);
	ClearDepth();
}

void Graphics::ClearColor(float red, float green, float blue) noexcept
{
	const float color[] = { red, green, blue };
	Context->ClearRenderTargetView(RenderTargetView.Get(), color);
}

void Graphics::ClearDepth() noexcept
{
	Context->ClearDepthStencilView(DepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}

void Graphics::DrawScene()
{
}

const Microsoft::WRL::ComPtr<ID3D11Device>& Graphics::GetDevice() const
{
	return Device;
}

Camera& Graphics::GetCamera()
{
	return GraphicsCamera;
}

const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& Graphics::GetContext() const
{
	return Context;
}
