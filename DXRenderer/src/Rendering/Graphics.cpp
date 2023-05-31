#include "Graphics.h"
#include "Core/Exception.h"

#include <wrl.h>
#include <d3dcompiler.h>
#include <filesystem>
#include <source_location>

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

	const float vertices[] =
	{
		0.0f, 0.5f,
		0.5f, -0.5f,
		-0.5f, -0.5f
	};

	WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.ByteWidth = sizeof(vertices);
	vertexBufferDesc.StructureByteStride = 2 * sizeof(float);

	D3D11_SUBRESOURCE_DATA subResourceData;
	subResourceData.pSysMem = vertices;
	Device->CreateBuffer(&vertexBufferDesc, &subResourceData, &vertexBuffer);

	const UINT stride = 2 * sizeof(float);
	const UINT offset = 0;
	Context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

	WRL::ComPtr<ID3D11PixelShader> pixelShader;
	WRL::ComPtr<ID3DBlob> blob;

	std::string currentDir = std::string(std::source_location::current().file_name());
	currentDir = currentDir.substr(0, currentDir.find_last_of("\\/"));
	D3DReadFileToBlob(std::wstring(std::wstring(currentDir.begin(), currentDir.end()) + L"\\Shaders\\PixelShader.cso").c_str(), &blob);
	Device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pixelShader);

	Context->PSSetShader(pixelShader.Get(), nullptr, 0);

	WRL::ComPtr<ID3D11VertexShader> vertexShader;
	D3DReadFileToBlob(std::wstring(std::wstring(currentDir.begin(), currentDir.end()) + L"\\Shaders\\VertexShader.cso").c_str(), &blob);
	Device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &vertexShader);

	Context->VSSetShader(vertexShader.Get(), nullptr, 0);

	WRL::ComPtr<ID3D11InputLayout> inputLayout;
	const D3D11_INPUT_ELEMENT_DESC desc[] =
	{
		{"Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	Device->CreateInputLayout(desc, (UINT)std::size(desc), blob->GetBufferPointer(), blob->GetBufferSize(), &inputLayout);
	
	Context->IASetInputLayout(inputLayout.Get());
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
