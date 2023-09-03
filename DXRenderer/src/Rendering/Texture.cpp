#include "Core\Exception.h"
#include "Rendering\CurrentGraphicsContext.h"
#include "RenderTarget.h"
#include "Texture.h"

#include <wrl.h>
#include <source_location>
#include <filesystem>

Sampler::Sampler(uint32_t slot, SamplerInitializer init)
	:Slot(slot)
{
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = init.EnableAnisotropic ? D3D11_FILTER_ANISOTROPIC : D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = init.Reflect ? D3D11_TEXTURE_ADDRESS_MIRROR : D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = init.Reflect ? D3D11_TEXTURE_ADDRESS_MIRROR : D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = init.Reflect ? D3D11_TEXTURE_ADDRESS_MIRROR : D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	CurrentGraphicsContext::Device()->CreateSamplerState(&samplerDesc, &SamplerID);
}

inline void Sampler::Bind() const
{
	CurrentGraphicsContext::Context()->PSSetSamplers(Slot, 1, SamplerID.GetAddressOf());
}

ShadowSampler::ShadowSampler(uint32_t slot)
	:Slot(slot)
{
	D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };

	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

	CurrentGraphicsContext::Device()->CreateSamplerState(&samplerDesc, &SamplerID);
}

void ShadowSampler::Bind() const
{
	CurrentGraphicsContext::Context()->PSSetSamplers(Slot, 1, SamplerID.GetAddressOf());
}

Texture::Texture(const std::string& filename, uint32_t slot)
	:Slot(slot), TextureSampler{ slot, SamplerInitializer{ false, false } }
{
	auto filepath = std::filesystem::current_path().parent_path().string() + "\\Content\\" + filename;
	wchar_t wideName[512];
	mbstowcs_s(nullptr, wideName, filepath.c_str(), _TRUNCATE);
	GRAPHICS_ASSERT(DirectX::LoadFromWICFile(wideName, DirectX::WIC_FLAGS_NONE, nullptr, Image));
	
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = GetWidth();
	textureDesc.Height = GetHeight();
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = Image.GetMetadata().format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	CurrentGraphicsContext::Device()->CreateTexture2D(&textureDesc, nullptr, &TextureID);
	CurrentGraphicsContext::Context()->UpdateSubresource(TextureID.Get(), 0, nullptr, Image.GetPixels(),
														 Image.GetImage(0,0,0)->rowPitch, 0);

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

CubeTexture::CubeTexture(uint32_t slot)
	:Slot(slot), TextureSampler{}
{
	auto filepath = std::filesystem::current_path().parent_path().string() + "\\Content\\Img\\Skybox\\";

	std::vector<DirectX::ScratchImage> images(6);
	for (size_t i = 0; i < images.size(); i++)
	{
		auto shortName = filepath + std::to_string(i) + ".png";
		wchar_t wideName[512];
		mbstowcs_s(nullptr, wideName, shortName.c_str(), _TRUNCATE);
		GRAPHICS_ASSERT(DirectX::LoadFromWICFile(wideName, DirectX::WIC_FLAGS_NONE, nullptr, images[i]));
	}

	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = images[0].GetMetadata().width;
	textureDesc.Height = images[0].GetMetadata().height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 6;
	textureDesc.Format = images[0].GetMetadata().format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	D3D11_SUBRESOURCE_DATA subResourceData[6];
	for (size_t i = 0; i < 6; i++)
	{
		subResourceData[i].pSysMem = images[i].GetPixels();
		subResourceData[i].SysMemPitch = images[i].GetImage(0, 0, 0)->rowPitch;
		subResourceData[i].SysMemSlicePitch = 0;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	GRAPHICS_ASSERT(CurrentGraphicsContext::Device()->CreateTexture2D(&textureDesc, subResourceData, &texture));

	D3D11_SHADER_RESOURCE_VIEW_DESC sourceDesc{};
	sourceDesc.Format = textureDesc.Format;
	sourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	sourceDesc.Texture2D.MostDetailedMip = 0;
	sourceDesc.Texture2D.MipLevels = 1;
	GRAPHICS_ASSERT(CurrentGraphicsContext::Device()->CreateShaderResourceView(texture.Get(), &sourceDesc, &TextureView));
}

void CubeTexture::Bind() const
{
	CurrentGraphicsContext::Context()->PSSetShaderResources(Slot, 1, TextureView.GetAddressOf());
	TextureSampler.Bind();
}