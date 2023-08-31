#include "PassExtensions.h"
#include "Rendering/RenderTarget.h"
#include "Rendering/Buffer.h"
#include "Rendering/Shader.h"
#include "Rendering/State.h"
#include "Rendering/Texture.h"

ResourcesPass::ResourcesPass(std::string&& name)
	:Pass(std::move(name))
{}

ResourcesPass::ResourcesPass(std::string && name, GPUObjectBase && resources)
	: Pass(std::move(name)), Resources(std::move(resources))
{}

void ResourcesPass::Bind() const
{
	Pass::Bind();
	Resources.Bind();
}

void ResourcesPass::Validate()
{
	if (!RTarget && !DStencil)
		throw std::runtime_error("No valid resource available. At least one valid resource between a DepthStencil buffer and a RenderTargetView required");

	Pass::Validate();
}

ClearPass::ClearPass(std::string&& name)
	:Pass(std::move(name))
{
	Register<PassInput<RenderTarget>>("renderTarget", RTarget);
	Register<PassInput<DepthStencil>>("depthStencil", DStencil);
	Register<PassOutput<RenderTarget>>("renderTarget", RTarget);
	Register<PassOutput<DepthStencil>>("depthStencil", DStencil);
}

void ClearPass::Execute() const
{
	RTarget->Clear();
	DStencil->Clear();
}

FullScreenPass::FullScreenPass(std::string&& name)
	:ResourcesPass(std::move(name))
{
	if (Initialized) return;

	std::vector<DirectX::XMFLOAT2> vertices = { { -1, 1 },{ 1, 1 },{ -1, -1 },{ 1, -1 } };
	std::vector<unsigned short> indices = { 0, 1, 2, 1, 3, 2 };

	BufferLayout layout{ { LayoutElement::ElementType::Position2 } };
	VertexBuffer vb("$FullScreenFilter", vertices, layout);

	VertexShader vs("FullScreenFilter");
	Add<InputLayout>("$FullScreenFilter", vb.GetLayout(), vs.GetBlob());
	Add<VertexShader>(vs);
	Add<VertexBuffer>(vb);
	Add<IndexBuffer>("$FullScreenFilter", indices);
	Add<Sampler>(0, SamplerInitializer{ false, true });
}

void FullScreenPass::Execute() const
{
	Bind();
	CurrentGraphicsContext::Context()->DrawIndexed(
		Resources.GetIndexBuffer()->GetCount(), 0, 0);
}

RenderQueuePass::RenderQueuePass(std::string&& name)
	:ResourcesPass(std::move(name)), Tasks{}
{}

RenderQueuePass::RenderQueuePass(std::string&& name, GPUObjectBase&& resources)
	:ResourcesPass(std::move(name), std::move(resources)), Tasks{}
{}

void RenderQueuePass::PushBack(Task task)
{
	Tasks.push_back(task);
}

void RenderQueuePass::Execute() const
{
	Bind();
	for (auto& task : Tasks)
		task.Execute();
}

void RenderQueuePass::Reset()
{
	Tasks.clear();
}

PhongPass::PhongPass(std::string&& name)
	:RenderQueuePass(std::move(name))
{
	Register<PassInput<RenderTarget>>("renderTarget", RTarget);
	Register<PassInput<DepthStencil>>("depthStencil", DStencil);
	Register<PassInput<DepthStencil>>("shadowMap", ShadowMap);
	Register<PassOutput<RenderTarget>>("renderTarget", RTarget);
	Register<PassOutput<DepthStencil>>("depthStencil", DStencil);
	Add<StencilState<DepthStencilMode::Off>>();
	Add<BlendState>("phongBlend", false);
	Add<ShadowSampler>(3);
}

void PhongPass::Execute() const
{
	ShadowMap->Bind();
	RenderQueuePass::Execute();
}

OutlineDrawPass::OutlineDrawPass(std::string&& name)
	:RenderQueuePass(std::move(name))
{
	Register<PassInput<RenderTarget>>("renderTarget", RTarget);
	Register<PassInput<DepthStencil>>("depthStencil", DStencil);
	Register<PassOutput<RenderTarget>>("renderTarget", RTarget);
	Register<PassOutput<DepthStencil>>("depthStencil", DStencil);

	Add<VertexShader>("colorInput");
	Add<PixelShader>("colorInput");
	Add<StencilState<DepthStencilMode::Mask>>();
	Add<RasterizerState>(false);
}

OutlineMaskPass::OutlineMaskPass(std::string&& name)
	:RenderQueuePass(std::move(name))
{
	Register<PassInput<DepthStencil>>("depthStencil", DStencil);
	Register<PassOutput<DepthStencil>>("depthStencil", DStencil);
	Add<VertexShader>("colorInput");
	Add<NullPixelShader>();
	Add<StencilState<DepthStencilMode::Write>>();
	Add<RasterizerState>(false);
}

BlurOutlineDrawPass::BlurOutlineDrawPass(std::string&& name, uint32_t width, uint32_t height)
	:RenderQueuePass(std::move(name))
{
	RTarget = MakeUnique<RenderTargetInput>(width / 2, height / 2);
	Add<VertexShader>("colorInput");
	Add<PixelShader>("colorInput");
	Add<StencilState<DepthStencilMode::Mask>>();
	Add<BlendState>("bluroutlinepass", false);
	Register<PassOutput<RenderTarget, true>>("scratchOut", RTarget);
}

void BlurOutlineDrawPass::Execute() const
{
	RTarget->Clear();
	RenderQueuePass::Execute();
}

HorizontalBlurPass::HorizontalBlurPass(std::string&& name, uint32_t width, uint32_t height)
	:FullScreenPass(std::move(name))
{
	Add<PixelShader>("FullScreenFilterOpt");
	Add<StencilState<DepthStencilMode::Off>>();
	Add<Sampler>(false);

	Register<PassInput<UniformPS<Kernel>>>("control", ConvKernel);
	Register<PassInput<UniformPS<BOOL>>>("direction", HorizontalFlag);
	Register<PassInput<RenderTarget>>("scratchIn", BlurScratchIn);

	RTarget = MakeShared<RenderTargetInput>(width / 2, height / 2);
	Register<PassOutput<RenderTarget, true>>("scratchOut", RTarget);
}

void HorizontalBlurPass::Execute() const
{
	HorizontalFlag->GetResourceRef() = true;
	HorizontalFlag->Bind();
	ConvKernel->Bind();
	BlurScratchIn->Bind();
	BlendState("$FullScreenFilter", true).Bind();
	FullScreenPass::Execute();
}

VerticalBlurPass::VerticalBlurPass(std::string&& name)
	:FullScreenPass(std::move(name))
{
	Add<PixelShader>("FullScreenFilterOpt");
	Add<StencilState<DepthStencilMode::Mask>>();

	Register<PassInput<UniformPS<Kernel>>>("control", ConvKernel);
	Register<PassInput<UniformPS<BOOL>>>("direction", HorizontalFlag);
	Register<PassInput<RenderTarget>>("scratchIn", BlurScratchIn);
	Register<PassInput<RenderTarget>>("renderTarget", RTarget);
	Register<PassInput<DepthStencil>>("depthStencil", DStencil);

	Register<PassOutput<RenderTarget>>("renderTarget", RTarget);
	Register<PassOutput<DepthStencil>>("depthStencil", DStencil);
}

void VerticalBlurPass::Execute() const
{
	HorizontalFlag->GetResourceRef() = false;
	HorizontalFlag->Bind();
	ConvKernel->Bind();
	BlurScratchIn->Bind();
	BlendState("$FullScreenFilter", true).Bind();
	FullScreenPass::Execute();
}

ShadowMappingPass::ShadowMappingPass(std::string&& name)
	:RenderQueuePass(std::move(name))
{
	DStencil = MakeUnique<DepthStencilInput>(CurrentGraphicsContext::GraphicsInfo->GetWidth(),
											 CurrentGraphicsContext::GraphicsInfo->GetHeight(),
											 3,
											 DepthStencilUse::ShadowDepth);
	Add<VertexShader>("ShadowMapUpdate");
	Add<NullPixelShader>();
	Add<StencilState<DepthStencilMode::Off>>();
	Add<BlendState>("shadowMapBL", false);

	Register<PassInput<ShadowRasterizerState>>("shadowRasterizer", ShadowRasterizer);
	Register<PassOutput<DepthStencil>>("map", DStencil);
}

void ShadowMappingPass::Execute() const
{
	ID3D11ShaderResourceView* const pNullTex = nullptr;
	CurrentGraphicsContext::Context()->PSSetShaderResources(3, 1, &pNullTex); // shadow map texture
	DStencil->Clear();
	RenderQueuePass::Execute();
	CurrentGraphicsContext::Context()->OMSetRenderTargets(0, nullptr, nullptr);
}
