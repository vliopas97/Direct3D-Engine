#include "Filter.h"
#include "CurrentGraphicsContext.h"
#include "Texture.h"
#include "State.h"

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
