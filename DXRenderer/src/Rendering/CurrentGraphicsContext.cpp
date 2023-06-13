#include "CurrentGraphicsContext.h"
#include "Graphics.h"

Graphics* CurrentGraphicsContext::GraphicsInfo = nullptr;

const Microsoft::WRL::ComPtr<ID3D11Device>& CurrentGraphicsContext::Device()
{
	const auto& device = GraphicsInfo->GetDevice();
	return device;
}

const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& CurrentGraphicsContext::Context()
{
	const auto& context = GraphicsInfo->GetContext();
	return context;
}
