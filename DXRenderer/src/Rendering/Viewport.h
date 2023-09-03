#pragma once

#include "Component.h"
#include <d3d11.h>

class Viewport : public Component
{
public:
	Viewport(uint32_t width, uint32_t height);
	void Bind() const override;

private:
	D3D11_VIEWPORT ViewportID{};
};
