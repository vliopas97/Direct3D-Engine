#pragma once

#include "Core\Core.h"
#include "Utilities.h"

#include <d3d11.h>
#include <wrl.h>
#include <optional>

enum class DepthStencilUse
{
	Standard,
	ShadowDepth
};

class DepthStencil
{
public:
	void BindBuffer() const;
	void BindBuffer(class RenderTarget& renderTarget);
	void Clear();
	virtual void Bind() {};

protected:
	DepthStencil(uint32_t width, uint32_t height, bool isShaderResource, DepthStencilUse use);
	DepthStencil(Microsoft::WRL::ComPtr<ID3D11Texture2D> texture, uint32_t face);

protected:
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilView;

	friend class RenderTarget;
	friend class Graphics;
};

class DepthStencilInput : public DepthStencil
{
public:
	DepthStencilInput(uint32_t width, uint32_t height, uint32_t slot = 0,
					  DepthStencilUse use = DepthStencilUse::Standard);
	void Bind() override;

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShaderResourceView;
	uint32_t Slot;
};

class DepthStencilOutput : public DepthStencil
{
public:
	DepthStencilOutput(uint32_t width, uint32_t height);
	DepthStencilOutput(Microsoft::WRL::ComPtr<ID3D11Texture2D> texture, uint32_t face);
};

class RenderTarget
{
public:
	virtual void BindBuffer() const;
	void BindBuffer(DepthStencil& depthStencil) const;
	void Clear() const;
	void Clear(const std::array<float, 4>& color) const;
	virtual void Bind() const {};
	
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
	void Bind() const override;

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureView;
	uint32_t Slot;
};

class RenderTargetOutput : public RenderTarget
{
public:
	RenderTargetOutput(ID3D11Texture2D* texture);
	void Bind() const override;
};