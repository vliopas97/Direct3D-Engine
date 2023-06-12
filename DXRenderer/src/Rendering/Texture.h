#pragma once

#include "Core\Core.h"
#include <d3d11.h>
#include <string>

class Component
{
public:
	virtual void Bind() const = 0;
};

class ComponentGroup : public Component
{
public:
	void Bind() const override;

	void Add(UniquePtr<Component> component);

	void Add(ComponentGroup componentGroup);

private:
	std::vector<UniquePtr<Component>> Components;
};

class Sampler
{
public:
	Sampler();

	void Bind(uint32_t slot = 0) const;
private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerID;
};

class Texture : public Component
{
public:
	Texture(const std::string& filename);
	
	inline uint32_t GetWidth() const { return Width; }
	inline uint32_t GetHeight() const { return Height; }

	void Bind() const override;

private:
	uint32_t Width, Height;
	Sampler TextureSampler;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> TextureID;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureView;
};