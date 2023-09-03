#pragma once

#include "Buffer.h"
#include "RenderTarget.h"

#include <d3d11.h>
#include <DirectXTex.h>

class CubeTextureDepth : public BufferBase
{
public:
	CubeTextureDepth(uint32_t size, uint32_t slot = 0);
	void Bind() const override;
	void Unbind() const override;
	std::string GetID() const override { return "GlobalShadowMap"; }
	SharedPtr<DepthStencilOutput> operator[](size_t i);

protected:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureView;
	std::vector < SharedPtr<class DepthStencilOutput>> DepthBuffers;

private:
	uint32_t Slot;
};
