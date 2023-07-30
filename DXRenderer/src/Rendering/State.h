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