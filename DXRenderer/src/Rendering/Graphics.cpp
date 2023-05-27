#include "Graphics.h"
#include "Core/Exception.h"

Graphics::Graphics(HWND windowHandle)
{
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

	ID3D11Device* device = nullptr;
	IDXGISwapChain* swapChain = nullptr;
	ID3D11DeviceContext* context = nullptr;

	GRAPHICS_ASSERT(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&swapChain,
		&device,
		nullptr,
		&context
		));

	Device = UniquePtr<ID3D11Device>(device, Deleter<ID3D11Device>);
	SwapChain = UniquePtr<IDXGISwapChain>(swapChain, Deleter<IDXGISwapChain>);
	Context = UniquePtr<ID3D11DeviceContext>(context, Deleter<ID3D11DeviceContext>);

	ID3D11Resource* backBuffer = nullptr;
	GRAPHICS_ASSERT(SwapChain->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(&backBuffer)));
	ID3D11RenderTargetView* target = nullptr;
	GRAPHICS_ASSERT(Device->CreateRenderTargetView(backBuffer, nullptr, &target));
	backBuffer->Release();

	RenderTargetView = UniquePtr<ID3D11RenderTargetView>(target, Deleter<ID3D11RenderTargetView>);
}

void Graphics::SwapBuffers()
{
	HRESULT result;
	if (FAILED(result = SwapChain->Present(1u, 0u)))
	{
		if (result == DXGI_ERROR_DEVICE_REMOVED)
		{
			throw GRAPHICS_DEVICE_REMOVED_EXCEPTION(Device->GetDeviceRemovedReason());
		}
		else
		{
			GRAPHICS_ASSERT(result);
		}
	}
}

void Graphics::ClearColor(float red, float green, float blue) noexcept
{
	const float color[] = { red, green, blue };
	Context->ClearRenderTargetView(RenderTargetView.get(), color);
}
