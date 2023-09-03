#include "Viewport.h"

#include "CurrentGraphicsContext.h"
#include "Graphics.h"

Viewport::Viewport(uint32_t width, uint32_t height)
{
	ViewportID.Width = width;
	ViewportID.Height = height;
	ViewportID.MinDepth = 0.0f;
	ViewportID.MaxDepth = 1.0f;
	ViewportID.TopLeftX = 0.0f;
	ViewportID.TopLeftY = 0.0f;
}

void Viewport::Bind() const
{
	CurrentGraphicsContext::Context()->RSSetViewports(1, &ViewportID);
}
