#pragma once

#include "Core\Core.h"
#include "Component.h"

#include <d3d11.h>
#include <DirectXTex.h>
#include <string>

class Sampler
{
public:
	Sampler(uint32_t slot = 0);

	void Bind() const;
private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerID;
	uint32_t Slot;
};

class Texture : public Component
{
public:
	Texture(const std::string& filename, uint32_t slot = 0);
	
	inline uint32_t GetWidth() const { return (uint32_t)Image.GetMetadata().width; }
	inline uint32_t GetHeight() const { return (uint32_t)Image.GetMetadata().height; }

	void Bind() const override;
	inline bool HasAlpha() const { return !Image.IsAlphaAllOpaque(); }

private:
	uint32_t Slot;
	Sampler TextureSampler;
	DirectX::ScratchImage Image;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> TextureID;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureView;
};