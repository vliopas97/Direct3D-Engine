#include "State.h"
#include "CurrentGraphicsContext.h"

BlendState::BlendState(const std::string& tag, bool blendingEnabled)
	:Tag(tag), IsBlendingEnabled(blendingEnabled)
{
	D3D11_BLEND_DESC blendDesc{};
	auto& brt = blendDesc.RenderTarget[0];

	if (IsBlendingEnabled)
	{
		brt.BlendEnable = TRUE;
		brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		brt.BlendOp = D3D11_BLEND_OP_ADD;
		brt.SrcBlendAlpha = D3D11_BLEND_ZERO;
		brt.DestBlendAlpha = D3D11_BLEND_ZERO;
		brt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		brt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
	else
	{
		brt.BlendEnable = FALSE;
		brt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}

	CurrentGraphicsContext::Device()->CreateBlendState(&blendDesc, &StateID);
}

void BlendState::Bind() const
{
	CurrentGraphicsContext::Context()->OMSetBlendState(StateID.Get(), nullptr, 0xFFFFFFFFu);
}

void BlendState::Unbind() const
{
	CurrentGraphicsContext::Context()->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFFu);
}

std::string BlendState::GetID() const
{
	return std::string(typeid(BlendState).name()) + "#" + Tag;
}

RasterizerState::RasterizerState(bool renderBothSides)
	:RenderBothSides(renderBothSides)
{
	D3D11_RASTERIZER_DESC rasterizerDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
	rasterizerDesc.CullMode = RenderBothSides ? D3D11_CULL_NONE : D3D11_CULL_BACK;

	GRAPHICS_ASSERT(CurrentGraphicsContext::Device()->CreateRasterizerState(&rasterizerDesc, &StateID));
}

void RasterizerState::Bind() const
{
	CurrentGraphicsContext::Context()->RSSetState(StateID.Get());
}

void RasterizerState::Unbind() const
{
	CurrentGraphicsContext::Context()->RSSetState(nullptr);
}

std::string RasterizerState::GetID() const
{
	return std::string(typeid(RasterizerState).name()) + std::to_string(RenderBothSides);
}

ShadowRasterizerState::ShadowRasterizerState()
	:ShadowRasterizerState(40, 4.5f, 1.0f)
{}

ShadowRasterizerState::ShadowRasterizerState(int depthBias, float slopeBias, float clamp)
	:DepthBias(depthBias), SlopeBias(slopeBias), Clamp(clamp)
{
	D3D11_RASTERIZER_DESC rasterizerDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
	rasterizerDesc.DepthBias = depthBias;
	rasterizerDesc.SlopeScaledDepthBias = slopeBias;
	rasterizerDesc.DepthBiasClamp = clamp;

	GRAPHICS_ASSERT(CurrentGraphicsContext::Device()->CreateRasterizerState(&rasterizerDesc, &StateID));
}

void ShadowRasterizerState::Bind() const
{
	CurrentGraphicsContext::Context()->RSSetState(StateID.Get());
}

void ShadowRasterizerState::Unbind() const
{
	CurrentGraphicsContext::Context()->RSSetState(nullptr);
}

std::string ShadowRasterizerState::GetID() const
{
	return std::string(typeid(ShadowRasterizerState).name()) + std::to_string(DepthBias) + std::to_string(SlopeBias) + std::to_string(Clamp);
}
