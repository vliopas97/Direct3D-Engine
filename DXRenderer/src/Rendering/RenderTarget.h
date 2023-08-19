#pragma once

#include "Core\Core.h"
#include "Utilities.h"

#include <d3d11.h>
#include <wrl.h>

class BufferResource
{
public:
	virtual void Bind() const = 0;
};

class DepthStencil : public BufferResource
{
public:
	void Bind() const;
	void Bind(class RenderTarget& renderTarget);
	void Clear();

protected:
	DepthStencil(uint32_t width, uint32_t height, bool isShaderResource = true);

protected:
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilView;

	friend class RenderTarget;
	friend class Graphics;
};

class DepthStencilInput : public DepthStencil
{
public:
	DepthStencilInput(uint32_t width, uint32_t height, uint32_t slot = 0);
	void BindAsTexture();

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShaderResourceView;
	uint32_t Slot;
};

class DepthStencilOutput : public DepthStencil
{
public:
	DepthStencilOutput(uint32_t width, uint32_t height);
};

class RenderTarget : public BufferResource
{
public:
	void Bind() const;
	void Bind(DepthStencil& depthStencil) const;
	void Clear() const;
	void Clear(const std::array<float, 4>& color) const;
	
protected:
	RenderTarget(uint32_t width, uint32_t height);
	RenderTarget(ID3D11Texture2D* texture);

protected:
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RenderTargetView;

	uint32_t Width, Height;
};

class RenderTargetInput : public RenderTarget
{
public:
	RenderTargetInput(uint32_t width, uint32_t height, uint32_t slot = 0);
	void BindAsTexture() const;

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureView;
	uint32_t Slot;
};

class RenderTargetOutput : public RenderTarget
{
public:
	RenderTargetOutput(ID3D11Texture2D* texture);
};