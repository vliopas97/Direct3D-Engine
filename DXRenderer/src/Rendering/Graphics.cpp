#include "Graphics.h"
#include "Core/Exception.h"

#include <d3dcompiler.h>
#include <source_location>

#include "Buffer.h"
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

void Graphics::DrawTriangle()
{
	namespace WRL = Microsoft::WRL;
	CurrentGraphicsContext::GraphicsInfo = this;

	VertexElement vertices[] =
	{
		{-1.0f, -1.0f, -1.0f, 0, 0, 255, 1},
		{ 1.0f, -1.0f, -1.0f, 0, 255, 255, 1},
		{-1.0f,  1.0f, -1.0f, 255, 0, 0, 1},
		{ 1.0f,  1.0f, -1.0f, 255, 0, 255, 1},
		{-1.0f, -1.0f,  1.0f, 0, 255, 255, 1},
		{ 1.0f, -1.0f,  1.0f, 255, 0, 255, 1},
		{-1.0f,  1.0f,  1.0f, 0, 255, 255, 1},
		{ 1.0f,  1.0f,  1.0f, 0, 255, 0, 1}
	};

	unsigned short indices[] =
	{
		0,2,1, 2,3,1,
		1,3,5, 3,7,5,
		2,6,3, 3,6,7,
		4,5,7, 4,7,6,
		0,4,2, 2,4,6,
		0,1,4, 1,5,4
	};

	VertexBuffer vertexBuffer(vertices, std::size(vertices));
	vertexBuffer.Bind();

	IndexBuffer indexBuffer(indices, std::size(indices));
	indexBuffer.Bind();

	float angle = 40.0f;
	VSConstantBuffer<DirectX::XMMATRIX> model(DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(angle))
											*DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(angle))
											*DirectX::XMMatrixTranslation(0, 0, 4)
											*DirectX::XMMatrixPerspectiveFovLH(5.0f, 16.0f/9.0f, 0.1f, 10.0f));
	model.Bind();

	ShaderGroup shaderGroup;

	UniquePtr<VertexShader> vertexShader = MakeUnique<VertexShader>("VertexShader");
	UniquePtr<PixelShader>pixelShader = MakeUnique<PixelShader>("PixelShader");

	shaderGroup.AddShader(std::move(vertexShader));
	shaderGroup.AddShader(std::move(pixelShader));
	shaderGroup.Bind();

	PSConstantBuffer<FaceColors> fColors(
		{{
			{1.0f,0.0f,1.0f, 1.0f},
			{1.0f,0.0f,0.0f, 1.0f},
			{0.0f,1.0f,0.0f, 1.0f},
			{0.0f,0.0f,1.0f, 1.0f},
			{1.0f,1.0f,0.0f, 1.0f},
			{0.0f,1.0f,1.0f, 1.0f},
		}});
	fColors.Bind();

	vertexBuffer.AddLayoutElement({ "Position", LayoutElement::DataType::Float3 }).
				 AddLayoutElement({ "Color", LayoutElement::DataType::UChar4Norm });
	vertexBuffer.CreateLayout(shaderGroup.GetBlob(ShaderType::Vertex));

	Context->DrawIndexed((UINT)std::size(indices), 0, 0);

	// Second cube for depth testing
	angle = -35.0f;
	model = DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(angle))
		* DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(angle))
		* DirectX::XMMatrixTranslation(-2, 0, 6)
		* DirectX::XMMatrixPerspectiveFovLH(5.0f, 16.0f / 9.0f, 0.1f, 10.0f);
	model.Bind();
	Context->DrawIndexed((UINT)std::size(indices), 0, 0);

	// Create Depth Buffer
	DepthBuffer depthBuffer(DepthStencilView);
	depthBuffer.Bind();

	Context->OMSetRenderTargets(1, RenderTargetView.GetAddressOf(), DepthStencilView.Get());
	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = 1920.0f;
	viewport.Height = 1080.0f;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	Context->RSSetViewports(1, &viewport);
}

const Microsoft::WRL::ComPtr<ID3D11Device>& Graphics::GetDevice() const
{
	return Device;
}

const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& Graphics::GetContext() const
{
	return Context;
}
