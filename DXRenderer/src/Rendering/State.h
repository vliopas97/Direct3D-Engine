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

class ShadowRasterizerState : public State
{
public:
	ShadowRasterizerState();
	ShadowRasterizerState(int depthBias, float slopeBias, float clamp);

	void Bind() const override;
	void Unbind() const override;

	std::string GetID() const;

private:
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> StateID;
	float DepthBias, SlopeBias, Clamp;
};

enum class DepthStencilMode
{
	Off,
	Write,
	Mask,
	Skybox
};

template<DepthStencilMode Mode = DepthStencilMode::Off>
class StencilState : public State
{
public:
	StencilState()
	{
		Tag = "Off";
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };;

		if constexpr (Mode == DepthStencilMode::Write)
		{
			depthStencilDesc.DepthEnable = FALSE;
			depthStencilDesc.StencilEnable = TRUE;
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthStencilDesc.StencilWriteMask = 0xFF;
			depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;

			Tag = "Write";
		}
		else if constexpr (Mode == DepthStencilMode::Mask)
		{
			depthStencilDesc.DepthEnable = FALSE;
			depthStencilDesc.StencilEnable = TRUE;
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthStencilDesc.StencilWriteMask = 0xFF;
			depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
			depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

			Tag = "Mask";
		}
		else if constexpr (Mode == DepthStencilMode::Skybox)
		{
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

			Tag = "Skybox";
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

