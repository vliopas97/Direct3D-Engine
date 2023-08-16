#include "RenderTarget.h"
#include "CurrentGraphicsContext.h"

DepthStencil::DepthStencil(uint32_t width, uint32_t height)
{
	D3D11_TEXTURE2D_DESC descDepth{};
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1u;
	descDepth.ArraySize = 1u;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1u;
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	CurrentGraphicsContext::Device()->CreateTexture2D(&descDepth, nullptr, &DepthStencilID);
	CurrentGraphicsContext::Device()->CreateDepthStencilView(DepthStencilID.Get(), nullptr, &DepthStencilView);
}

void DepthStencil::Bind() const
{
	CurrentGraphicsContext::Context()->OMSetRenderTargets(0, nullptr, DepthStencilView.Get());
}

void DepthStencil::Clear()
{
	CurrentGraphicsContext::Context()->ClearDepthStencilView(DepthStencilView.Get(),
															 D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
															 1.0f, 0);
}

RenderTarget::RenderTarget(uint32_t width, uint32_t height)
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

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	CurrentGraphicsContext::Device()->CreateShaderResourceView(texture.Get(), &shaderResourceViewDesc, &TextureView);

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D = D3D11_TEX2D_RTV{ 0 };
	CurrentGraphicsContext::Device()->CreateRenderTargetView(texture.Get(), &renderTargetViewDesc, &RenderTargetView);
}

void RenderTarget::BindAsTexture(uint32_t slot) const
{
	CurrentGraphicsContext::Context()->PSSetShaderResources(slot, 1, TextureView.GetAddressOf());
}

void RenderTarget::Bind() const
{
	CurrentGraphicsContext::Context()->OMSetRenderTargets(1, RenderTargetView.GetAddressOf(), nullptr);
}

void RenderTarget::Bind(DepthStencil& depthStencil) const
{
	CurrentGraphicsContext::Context()->OMSetRenderTargets(1, RenderTargetView.GetAddressOf(), depthStencil.DepthStencilView.Get());
}

void RenderTarget::Clear() const
{
	CurrentGraphicsContext::Context()->ClearRenderTargetView(RenderTargetView.Get(), backgroundColor.data());
}

void RenderTarget::Clear(const std::array<float, 4>& color) const
{
	CurrentGraphicsContext::Context()->ClearRenderTargetView(RenderTargetView.Get(), color.data());
}
