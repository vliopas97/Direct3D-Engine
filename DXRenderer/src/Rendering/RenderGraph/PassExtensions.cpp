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
	Add<BlendState>("$FullScreenFilter", true);
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
	Register<PassOutput<RenderTarget>>("renderTarget", RTarget);
	Register<PassOutput<DepthStencil>>("depthStencil", DStencil);
	Add<StencilState<DepthStencilMode::Off>>();
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
