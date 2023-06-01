#include "Graphics.h"
#include "Core/Exception.h"

#include <wrl.h>
#include <d3dcompiler.h>
#include <source_location>

#include "Buffer.h"
#include "CurrentGraphicsContext.h"
#include "Shader.h"

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

	UINT swapCreateFlags = 0u;
	#ifndef NDEBUG
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	ID3D11Device* device = nullptr;
	IDXGISwapChain* swapChain = nullptr;
	ID3D11DeviceContext* context = nullptr;

	GRAPHICS_ASSERT(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		swapCreateFlags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&swapChain,
		&device,
		nullptr,
		&context
		));

	Device = UniquePtrCustomDeleter<ID3D11Device>(device, Deleter<ID3D11Device>);
	SwapChain = UniquePtrCustomDeleter<IDXGISwapChain>(swapChain, Deleter<IDXGISwapChain>);
	Context = UniquePtrCustomDeleter<ID3D11DeviceContext>(context, Deleter<ID3D11DeviceContext>);

	ID3D11Resource* backBuffer = nullptr;
	GRAPHICS_ASSERT(SwapChain->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(&backBuffer)));
	ID3D11RenderTargetView* target = nullptr;
	GRAPHICS_ASSERT(Device->CreateRenderTargetView(backBuffer, nullptr, &target));
	backBuffer->Release();

	RenderTargetView = UniquePtrCustomDeleter<ID3D11RenderTargetView>(target, Deleter<ID3D11RenderTargetView>);
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

void Graphics::ClearColor(float red, float green, float blue) noexcept
{
	const float color[] = { red, green, blue };
	Context->ClearRenderTargetView(RenderTargetView.get(), color);
}

void Graphics::DrawTriangle()
{
	namespace WRL = Microsoft::WRL;

	float vertices[] =
	{
		0.0f, 0.5f,
		0.5f, -0.5f,
		-0.5f, -0.5f
	};

	CurrentGraphicsContext::GraphicsInfo = this;
	VertexBuffer vertexBuffer(vertices, std::size(vertices));
	vertexBuffer.Bind();

	VertexShader vertexShader("VertexShader");
	vertexShader.Bind();

	PixelShader pixelShader("PixelShader");
	pixelShader.Bind();

	BufferLayout layout;
	layout.AddElement({ "Position", LayoutElement::DataType::Float2 });
	vertexBuffer.SetLayout(layout);
	vertexBuffer.CreateLayout(vertexShader.GetBlob());

	ID3D11RenderTargetView* const ptr = reinterpret_cast<ID3D11RenderTargetView* const>(RenderTargetView.get());
	EXCEPTION_WRAP(Context->OMSetRenderTargets(1, &ptr, nullptr););
	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = 1920.0f;
	viewport.Height = 1080.0f;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	Context->RSSetViewports(1, &viewport);

	Context->Draw((UINT)std::size(vertices), 0);
}

const UniquePtrCustomDeleter<ID3D11Device>& Graphics::GetDevice() const
{
	return Device;
}

const UniquePtrCustomDeleter<ID3D11DeviceContext>& Graphics::GetContext() const
{
	return Context;
}
