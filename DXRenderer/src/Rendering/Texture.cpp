#include "Core\Exception.h"
#include "Rendering\CurrentGraphicsContext.h"
#include "stb_image.h"
#include "Texture.h"

#include <wrl.h>
#include <source_location>

inline Sampler::Sampler()
{
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	CurrentGraphicsContext::Device()->CreateSamplerState(&samplerDesc, &SamplerID);
}

inline void Sampler::Bind(uint32_t slot) const
{
	CurrentGraphicsContext::Context()->PSSetSamplers(slot, 1, SamplerID.GetAddressOf());
}

Texture::Texture(const std::string& filename)
:TextureSampler{}
{
	int width, height, channels;
	int desiredChannels = 4;
	stbi_uc* data;

	auto filepath = std::string(std::source_location::current().file_name());
	filepath = filepath.substr(0, filepath.find_last_of("\\/")) + "\\Img\\" + filename;

	data = stbi_load(filepath.c_str(), &width, &height, &channels, desiredChannels);

	Height = height;
	Width = width;

	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = Width;
	textureDesc.Height = Height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = data;
	subresourceData.SysMemPitch = Width * desiredChannels;

	CurrentGraphicsContext::Device()->CreateTexture2D(&textureDesc, &subresourceData, &TextureID);

	D3D11_SHADER_RESOURCE_VIEW_DESC sourceDesc{};
	sourceDesc.Format = textureDesc.Format;
	sourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	sourceDesc.Texture2D.MostDetailedMip = 0;
	sourceDesc.Texture2D.MipLevels = 1;
	CurrentGraphicsContext::Device()->CreateShaderResourceView(TextureID.Get(), &sourceDesc, &TextureView);
}

inline void Texture::Bind() const
{
	CurrentGraphicsContext::Context()->PSSetShaderResources(0, 1, TextureView.GetAddressOf());
	TextureSampler.Bind();
}
