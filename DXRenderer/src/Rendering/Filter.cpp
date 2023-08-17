#include "Filter.h"
#include "CurrentGraphicsContext.h"
#include "Texture.h"

Filter::Filter(uint32_t width, uint32_t height)
	:RenderTargetID(width, height)
{
	std::vector<DirectX::XMFLOAT2> vertices = { { -1, 1 },{ 1, 1 },{ -1, -1 },{ 1, -1 } };
	std::vector<unsigned short> indices = { 0, 1, 2, 1, 3, 2 };

	BufferLayout layout{ { LayoutElement::ElementType::Position2 } };
	VertexBuffer vb("$FullScreenFilter", vertices, layout);

	VertexShader vs("FullScreenFilter");
	Add<InputLayout>("$FullScreenFilter", vb.GetLayout(), vs.GetBlob());
	Add<VertexShader>(vs);
	Add<PixelShader>("FullScreenFilter");
	Add<VertexBuffer>(vb);
	Add<IndexBuffer>("$FullScreenFilter", indices);
	Add<Sampler>(0, SamplerInitializer{ false, true });
	Add<BlendState>("$FullScreenFilter", true);
}

void Filter::Draw() const
{
	Bind();
	RenderTargetID.BindAsTexture();
	CurrentGraphicsContext::Context()->DrawIndexed(GetIndexBuffer()->GetCount(), 0, 0);
}

void Filter::Setup(DepthStencil& depthStencil) const
{
	RenderTargetID.Bind(depthStencil);
}

GaussFilter::GaussFilter(uint32_t width, uint32_t height, uint32_t radius, float sigma)
	:RT1{ width, height }, RT2{ width, height }, IsHorizontal{ TRUE }, ConvKernel{}
{
	InitKernel(radius, sigma);

	std::vector<DirectX::XMFLOAT2> vertices = { { -1, 1 },{ 1, 1 },{ -1, -1 },{ 1, -1 } };
	std::vector<unsigned short> indices = { 0, 1, 2, 1, 3, 2 };

	BufferLayout layout{ { LayoutElement::ElementType::Position2 } };
	VertexBuffer vb("$FullScreenFilter", vertices, layout);

	VertexShader vs("FullScreenFilter");
	Add<InputLayout>("$FullScreenFilter", vb.GetLayout(), vs.GetBlob());
	Add<VertexShader>(vs);
	Add<PixelShader>("FullScreenFilterOpt");
	Add<VertexBuffer>(vb);
	Add<IndexBuffer>("$FullScreenFilter", indices);
	Add<Sampler>(0, SamplerInitializer{ false, true });

	Add<UniformPS<Kernel>>("$FullScreenFilterKernel", ConvKernel);
	Add<UniformPS<BOOL>>("$FullScreenFilterBool", IsHorizontal, 1);
}

void GaussFilter::DrawHorizontal()
{
	Sampler(0, SamplerInitializer{ false, true }).Bind();
	Bind();
	IsHorizontal = TRUE;
	CurrentGraphicsContext::Context()->DrawIndexed(GetIndexBuffer()->GetCount(), 0, 0);
}

void GaussFilter::DrawVertical()
{
	BlendState{ "$FullScreenFilter", true }.Bind();
	Sampler(0, SamplerInitializer{ true, true }).Bind();
	Bind();
	RT2.BindAsTexture();
	IsHorizontal = FALSE;
	CurrentGraphicsContext::Context()->DrawIndexed(GetIndexBuffer()->GetCount(), 0, 0);
}

void GaussFilter::InitKernel(uint32_t radius, float sigma)
{
	ASSERT(radius <= MaxRadius);
	ConvKernel.Taps = radius * 2 + 1;
	float sum = 0.0f;
	for (size_t i = 0; i < ConvKernel.Taps; i++)
	{
		auto x = static_cast<float>((int)i - (int)radius);
		auto g = gauss(x, sigma);
		sum += g;
		ConvKernel.Coeff[i].x = g;
	}
	for (size_t i = 0; i < ConvKernel.Taps; i++)
	{
		ConvKernel.Coeff[i].x /= sum;
	}
}
