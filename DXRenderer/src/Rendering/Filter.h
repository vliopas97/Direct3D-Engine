#pragma once

#include "Core/Core.h"
#include "RenderTarget.h"
#include "Utilities.h"
#include "Buffer.h"
#include "Shader.h"
#include "State.h"

class Filter : public GPUObjectBase
{
public:
	Filter(uint32_t width, uint32_t height);

	void Draw() const;
	void Setup(DepthStencil& depthStencil) const;

	RenderTarget& GetRT() { return RenderTargetID; }
protected:
	using GPUObjectBase::Bind;

private:
	RenderTarget RenderTargetID;
};

class GaussFilter : public GPUObjectBase
{
public:
	GaussFilter(uint32_t width, uint32_t height, uint32_t radius = 7, float sigma = 2.6f);

	void DrawHorizontal();
	void DrawVertical();

private:
	void InitKernel(uint32_t radius, float sigma);

public:
	RenderTarget RT1, RT2;

private:
	static constexpr uint32_t MaxRadius = 15;
	struct Kernel
	{
		uint32_t Taps;
		float padding[3];
		DirectX::XMFLOAT4 Coeff[2 * MaxRadius + 1];
	};

	BOOL IsHorizontal;
	Kernel ConvKernel;
};