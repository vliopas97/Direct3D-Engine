#include "Core\Exception.h"
#include "Rendering\CurrentGraphicsContext.h"
#include "stb_image.h"
#include "Texture.h"

#include <wrl.h>
#include <source_location>
#include <filesystem>

inline Sampler::Sampler(uint32_t slot)
	:Slot(slot)
{
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	CurrentGraphicsContext::Device()->CreateSamplerState(&samplerDesc, &SamplerID);
}

inline void Sampler::Bind() const
{
	CurrentGraphicsContext::Context()->PSSetSamplers(Slot, 1, SamplerID.GetAddressOf());
}

Texture::Texture(const std::string& filename, uint32_t slot)
:Slot(slot), TextureSampler{Slot}
{
	int width, height, channels;
	int desiredChannels = 4;
	stbi_uc* data;

	auto filepath = std::filesystem::current_path().parent_path().string() + "\\Content\\" + filename;

	data = stbi_load(filepath.c_str(), &width, &height, &channels, desiredChannels);

	Height = height;
	Width = width;

	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = Width;
	textureDesc.Height = Height;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE |D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D10_RESOURCE_MISC_GENERATE_MIPS;

	CurrentGraphicsContext::Device()->CreateTexture2D(&textureDesc, nullptr, &TextureID);
	CurrentGraphicsContext::Context()->UpdateSubresource(TextureID.Get(), 0, nullptr, data, Width * channels * sizeof(char), 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC sourceDesc{};
	sourceDesc.Format = textureDesc.Format;
	sourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	sourceDesc.Texture2D.MostDetailedMip = 0;
	sourceDesc.Texture2D.MipLevels = -1;
	CurrentGraphicsContext::Device()->CreateShaderResourceView(TextureID.Get(), &sourceDesc, &TextureView);
	CurrentGraphicsContext::Context()->GenerateMips(TextureView.Get());
}

inline void Texture::Bind() const
{
	CurrentGraphicsContext::Context()->PSSetShaderResources(Slot, 1, TextureView.GetAddressOf());
	TextureSampler.Bind();
}
