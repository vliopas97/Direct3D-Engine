#pragma once

#include "Core/Core.h"
#include "RenderTarget.h"
#include "Utilities.h"
#include "Buffer.h"
#include "Shader.h"

class Filter : public GPUObjectBase
{
public:
	Filter(uint32_t width, uint32_t height);

	void Draw() const;
	void Setup(DepthStencil& depthStencil) const;

protected:
	using GPUObjectBase::Bind;

private:
	RenderTarget RenderTargetID;
};