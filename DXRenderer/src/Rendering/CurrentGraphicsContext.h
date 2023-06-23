#pragma once

#include "Graphics.h"

#include <d3d11.h>
#include <wrl.h>

class Graphics;

struct CurrentGraphicsContext
{
	static const Microsoft::WRL::ComPtr<ID3D11Device>& Device();
	static const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& Context();

	static Graphics* GraphicsInfo;
};
