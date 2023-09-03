#include "RenderTarget.h"
#include "CurrentGraphicsContext.h"

namespace
{

	DXGI_FORMAT GetFormat(DepthStencilUse use)
	{
		switch (use)
		{
		case DepthStencilUse::Standard:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case DepthStencilUse::ShadowDepth:
			return DXGI_FORMAT_D32_FLOAT;
		default:
			throw std::runtime_error("Depth stencil use mode unsupported");
		}
	}

	DXGI_FORMAT GetFormatTypeless(DepthStencilUse use)
	{
		switch (use)
		{
		case DepthStencilUse::Standard:
			return DXGI_FORMAT_R24G8_TYPELESS;
		case DepthStencilUse::ShadowDepth:
			return DXGI_FORMAT_R32_TYPELESS;
		default:
			throw std::runtime_error("Depth stencil use mode unsupported");
		}
	}

	DXGI_FORMAT GetFormatForTexture(DepthStencilUse use)
	{
		switch (use)
		{
		case DepthStencilUse::Standard:
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case DepthStencilUse::ShadowDepth:
			return DXGI_FORMAT_R32_FLOAT;
		default:
			throw std::runtime_error("Depth stencil use mode unsupported");
		}
	}
}

DepthStencil::DepthStencil(uint32_t width, 
						   uint32_t height,
						   bool isShaderResource,
						   DepthStencilUse use)
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilID;

	D3D11_TEXTURE2D_DESC descDepth{};
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1u;
	descDepth.ArraySize = 1u;
	descDepth.Format = GetFormatTypeless(use);
	descDepth.SampleDesc.Count = 1u;
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL 
		| (isShaderResource ? D3D11_BIND_SHADER_RESOURCE : 0);

	GRAPHICS_ASSERT(CurrentGraphicsContext::Device()->CreateTexture2D(&descDepth, nullptr, &depthStencilID));

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = GetFormat(use);
	depthStencilViewDesc.Flags = 0;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	GRAPHICS_ASSERT(CurrentGraphicsContext::Device()->CreateDepthStencilView(depthStencilID.Get(), &depthStencilViewDesc, &DepthStencilView));
}

DepthStencil::DepthStencil(Microsoft::WRL::ComPtr<ID3D11Texture2D> texture, uint32_t face)
{
	D3D11_TEXTURE2D_DESC textureDesc{};
	texture->GetDesc(&textureDesc);

	D3D11_DEPTH_STENCIL_VIEW_DESC dSVDesc{};
	dSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dSVDesc.Flags = 0;
	dSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	dSVDesc.Texture2DArray.MipSlice = 0;
	dSVDesc.Texture2DArray.ArraySize = 1;
	dSVDesc.Texture2DArray.FirstArraySlice = face;
	GRAPHICS_ASSERT(CurrentGraphicsContext::Device()->CreateDepthStencilView(texture.Get(), &dSVDesc, &DepthStencilView));
}

void DepthStencil::BindBuffer() const
{
	CurrentGraphicsContext::Context()->OMSetRenderTargets(0, nullptr, DepthStencilView.Get());
}

void DepthStencil::BindBuffer(RenderTarget& renderTarget)
{
	renderTarget.BindBuffer(*this);
}

void DepthStencil::Clear()
{
	CurrentGraphicsContext::Context()->ClearDepthStencilView(DepthStencilView.Get(),
															 D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
															 1.0f, 0);
}

DepthStencilInput::DepthStencilInput(uint32_t width, uint32_t height, uint32_t slot, DepthStencilUse use)
	: DepthStencil(width, height, true, use), Slot(slot)
{
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;
	DepthStencilView->GetResource(&resource);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = GetFormatForTexture(use);
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	GRAPHICS_ASSERT(CurrentGraphicsContext::Device()->CreateShaderResourceView(resource.Get(), &shaderResourceViewDesc, &ShaderResourceView))	;
}

void DepthStencilInput::Bind()
{
	(CurrentGraphicsContext::Context()->PSSetShaderResources(Slot, 1, ShaderResourceView.GetAddressOf()));
}

DepthStencilOutput::DepthStencilOutput(uint32_t width, uint32_t height)
	: DepthStencil(width, height, false, DepthStencilUse::Standard)
{}

DepthStencilOutput::DepthStencilOutput(Microsoft::WRL::ComPtr<ID3D11Texture2D> texture, uint32_t face)
	:DepthStencil(std::move(texture), face)
{}

RenderTarget::RenderTarget(uint32_t width, uint32_t height)
	:Width(width), Height(height)
{
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	CurrentGraphicsContext::Device()->CreateTexture2D(&textureDesc, nullptr, &texture);

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D = D3D11_TEX2D_RTV{ 0 };
	CurrentGraphicsContext::Device()->CreateRenderTargetView(texture.Get(), &renderTargetViewDesc, &RenderTargetView);
}

RenderTarget::RenderTarget(ID3D11Texture2D* texture)
{
	D3D11_TEXTURE2D_DESC textureDesc{};
	texture->GetDesc(&textureDesc);
	Width = textureDesc.Width;
	Height = textureDesc.Height;

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D = D3D11_TEX2D_RTV{ 0 };

	CurrentGraphicsContext::Device()->CreateRenderTargetView(texture, &renderTargetViewDesc, &RenderTargetView);
}

void RenderTarget::BindBuffer() const
{
	CurrentGraphicsContext::Context()->OMSetRenderTargets(1, RenderTargetView.GetAddressOf(), nullptr);

	D3D11_VIEWPORT viewport;
	viewport.Width = (float)Width;
	viewport.Height = (float)Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	CurrentGraphicsContext::Context()->RSSetViewports(1u, &viewport);
}

void RenderTarget::BindBuffer(DepthStencil& depthStencil) const
{
	CurrentGraphicsContext::Context()->OMSetRenderTargets(1, RenderTargetView.GetAddressOf(), depthStencil.DepthStencilView.Get());

	D3D11_VIEWPORT viewport;
	viewport.Width = (float)Width;
	viewport.Height = (float)Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	CurrentGraphicsContext::Context()->RSSetViewports(1u, &viewport);
}

void RenderTarget::Clear() const
{
	CurrentGraphicsContext::Context()->ClearRenderTargetView(RenderTargetView.Get(), backgroundColor.data());
}

void RenderTarget::Clear(const std::array<float, 4>& color) const
{
	CurrentGraphicsContext::Context()->ClearRenderTargetView(RenderTargetView.Get(), color.data());
}

RenderTargetInput::RenderTargetInput(uint32_t width, uint32_t height, uint32_t slot)
	:RenderTarget(width, height), Slot(slot)
{
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;
	RenderTargetView->GetResource(&resource);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	CurrentGraphicsContext::Device()->CreateShaderResourceView(resource.Get(), &shaderResourceViewDesc, &TextureView);
}

void RenderTargetInput::Bind() const
{
	CurrentGraphicsContext::Context()->PSSetShaderResources(Slot, 1, TextureView.GetAddressOf());
}

RenderTargetOutput::RenderTargetOutput(ID3D11Texture2D* texture)
	:RenderTarget(texture)
{}

void RenderTargetOutput::Bind() const
{
	ASSERT(false && "RenderTarget cannot be bound as an Output");
}
