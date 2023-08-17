#pragma once

#include "Core\Core.h"
#include "Utilities.h"

#include <d3d11.h>
#include <wrl.h>

class DepthStencil
{
public:
	DepthStencil(uint32_t width, uint32_t height);
	void Bind() const;
	void Clear();

private:
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> DepthStencilState;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> DepthStencilID;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilView;

	friend class RenderTarget;
	friend class Graphics;
};

class RenderTarget
{
public:
	RenderTarget(uint32_t width, uint32_t height);

	void BindAsTexture(uint32_t slot = 0) const;
	void Bind() const;
	void Bind(DepthStencil& depthStencil) const;
	void Clear() const;
	void Clear(const std::array<float, 4>& color) const;
private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureView;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RenderTargetView;

	uint32_t Width, Height;
};