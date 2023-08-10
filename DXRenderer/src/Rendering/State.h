#pragma once

#include "Core/Core.h"
#include "Buffer.h"

using State = BufferBase;

class BlendState : public State
{
public:
	BlendState(const std::string& tag, bool blendingEnabled);

	void Bind() const override;
	void Unbind() const override;
	std::string GetID() const override;

private:
	std::string Tag;
	bool IsBlendingEnabled;

	Microsoft::WRL::ComPtr<ID3D11BlendState> StateID;
};

class RasterizerState : public State
{
public:
	RasterizerState(bool renderBothSides);

	void Bind() const override;
	void Unbind() const override;
	std::string GetID() const override;

private:
	bool RenderBothSides;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> StateID;
};

enum class DepthStencilMode
{
	Off,
	Write,
	Mask
};

template<DepthStencilMode Mode = DepthStencilMode::Off>
class StencilState : public State
{
public:
	StencilState(const std::string& tag)
		:Tag(tag)
	{
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };;

		if constexpr (Mode == DepthStencilMode::Write)
		{
			depthStencilDesc.StencilEnable = TRUE;
			depthStencilDesc.StencilWriteMask = 0xFF;
			depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		}
		else if constexpr (Mode == DepthStencilMode::Mask)
		{
			depthStencilDesc.DepthEnable = FALSE;
			depthStencilDesc.StencilEnable = TRUE;
			depthStencilDesc.StencilWriteMask = 0xFF;
			depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
			depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		}

		CurrentGraphicsContext::Device()->CreateDepthStencilState(&depthStencilDesc, &StateID);
	}

	void Bind() const override
	{
		CurrentGraphicsContext::Context()->OMSetDepthStencilState(StateID.Get(), 0xFF);
	}

	void Unbind() const override
	{
		CurrentGraphicsContext::Context()->OMSetDepthStencilState(nullptr, 0xFF);
	}

	std::string GetID() const override
	{
		return std::string(typeid(StencilState).name()) + "#" + Tag;
	}
private:
	std::string Tag;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> StateID;
};

