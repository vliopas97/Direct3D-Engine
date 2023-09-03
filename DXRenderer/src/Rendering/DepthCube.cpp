#include "DepthCube.h"

CubeTextureDepth::CubeTextureDepth(uint32_t size, uint32_t slot)
	:Slot(slot)
{
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = size;
	textureDesc.Height = size;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 6;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	GRAPHICS_ASSERT(CurrentGraphicsContext::Device()->CreateTexture2D(&textureDesc, nullptr, &texture));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	GRAPHICS_ASSERT(CurrentGraphicsContext::Device()->CreateShaderResourceView(texture.Get(), &srvDesc, &TextureView));

	for (uint32_t face = 0; face < 6; face++)
		DepthBuffers.emplace_back(MakeShared<DepthStencilOutput>(texture, face));
}

void CubeTextureDepth::Bind() const
{
	CurrentGraphicsContext::Context()->PSSetShaderResources(Slot, 1, TextureView.GetAddressOf());
}

void CubeTextureDepth::Unbind() const
{
	CurrentGraphicsContext::Context()->PSSetShaderResources(Slot, 1, nullptr);
}

SharedPtr<DepthStencilOutput> CubeTextureDepth::operator[](size_t i)
{
	return DepthBuffers[i];
}
