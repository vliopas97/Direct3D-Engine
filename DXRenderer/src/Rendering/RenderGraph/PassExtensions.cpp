#include "PassExtensions.h"
#include "Rendering/Actors/Primitives.h"
#include "Rendering/Buffer.h"
#include "Rendering/Lights/PointLight.h"
#include "Rendering/RenderTarget.h"
#include "Rendering/Shader.h"
#include "Rendering/State.h"
#include "Rendering/Texture.h"
#include "Rendering/Viewport.h"

ResourcesPass::ResourcesPass(std::string&& name)
	:Pass(std::move(name))
{}

ResourcesPass::ResourcesPass(std::string&& name, GPUObjectBase&& resources)
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
	Register<PassInput<CubeTextureDepth>>("shadowMap", ShadowMap);
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

ShadowMappingPass::ShadowMappingPass(std::string&& name, const PointLight* pointLight)
	:RenderQueuePass(std::move(name)), LightSource(pointLight)
{
	DepthCube = MakeShared<CubeTextureDepth>(DepthDim, 3);
	Add<VertexShader>("ShadowMapUpdate");
	Add<StencilState<DepthStencilMode::Off>>();
	Add<BlendState>("shadowMapBL", false);
	Add<Viewport>(DepthDim, DepthDim);
	Add<ShadowRasterizerState>();

	ViewUniform = MakeUnique< UniformVS<DirectX::XMMATRIX>>("$shadowView", View, 3);
	ViewProjectionUniform = MakeUnique< UniformVS<DirectX::XMMATRIX>>("$shadowViewProj", ViewProjection, 4);

	Register<PassOutput<CubeTextureDepth>>("map", DepthCube);

	Projection = DirectX::XMMatrixPerspectiveFovLH(pi / 2.0f, 1.0f, 0.5f, 100.0f);

	CameraOrientation[0] = DirectX::XMFLOAT3{ 0, pi / 2.0f, 0 };
	CameraOrientation[1] = DirectX::XMFLOAT3{ 0, -pi / 2.0f, 0 };
	CameraOrientation[2] = DirectX::XMFLOAT3{ -pi / 2.0f, 0, 0 };
	CameraOrientation[3] = DirectX::XMFLOAT3{ pi / 2.0f, 0, 0 };
	CameraOrientation[4] = DirectX::XMFLOAT3{ 0, 0, 0 };
	CameraOrientation[5] = DirectX::XMFLOAT3{ 0, -pi, 0 };

	SetDepthBuffer(DepthCube->operator[](0));
}

void ShadowMappingPass::Execute() const
{
	using namespace DirectX;

	ID3D11ShaderResourceView* const pNullTex = nullptr;
	CurrentGraphicsContext::Context()->PSSetShaderResources(3, 1, &pNullTex); // shadow map texture

	auto position = LightSource->Position;
	for (size_t i = 0; i < 6; i++)
	{
		auto depthStencil = (*DepthCube)[i];
		depthStencil->Clear();
		SetDepthBuffer(std::move(depthStencil));

		auto Rotation = CameraOrientation[i];
		DirectX::XMMATRIX transform = XMMatrixRotationRollPitchYaw(-Rotation.x, -Rotation.y, Rotation.z) *
			XMMatrixTranslation(-position.x, -position.y, position.z);
		View = XMMatrixInverse(nullptr, transform);
		ViewProjection = View * Projection;

		ViewUniform->Bind();
		ViewProjectionUniform->Bind();
		RenderQueuePass::Execute();
	}

	View =XMMatrixTranslation(-position.x, -position.y, position.z);
	View = XMMatrixInverse(nullptr, View);
	ViewProjection = View * Projection;

	ViewUniform->Bind();
	ViewProjectionUniform->Bind();

	CurrentGraphicsContext::Context()->OMSetRenderTargets(0, nullptr, nullptr);
}

void ShadowMappingPass::SetLightSource(const PointLight* pointLight)
{
	LightSource = pointLight;
}

void ShadowMappingPass::SetDepthBuffer(SharedPtr<DepthStencil> depthStencil) const
{
	const_cast<ShadowMappingPass*>(this)->DStencil = depthStencil;
}

SkyboxPass::SkyboxPass(std::string&& name)
	:ResourcesPass(std::move(name))
{
	const auto tag = "$Skybox";

	Register<PassInput<RenderTarget>>("renderTarget", RTarget);
	Register<PassInput<DepthStencil>>("depthStencil", DStencil);
	Add<CubeTexture>();
	Add<StencilState<DepthStencilMode::Skybox>>();
	Add<RasterizerState>(true);

	auto data = Primitives::Cube::Create<Primitives::VertexElement>();
	Count = data.Indices.size();

	VertexShader vs("Skybox");
	VertexBuffer vb(tag, data.Vertices, BufferLayout{ { "Position", LayoutElement::DataType::Float3} });
	Add<InputLayout>(tag, vb.GetLayout(), vs.GetBlob());
	Add<UniformVS<DirectX::XMMATRIX>>(tag, CurrentGraphicsContext::GraphicsInfo->GetViewProjection());

	Add<VertexBuffer>(vb);
	Add<IndexBuffer>(tag, data.Indices);

	Add<VertexShader>(vs);
	Add<PixelShader>("Skybox");
	
	Register<PassOutput<RenderTarget>>("renderTarget", RTarget);
	Register<PassOutput<DepthStencil>>("depthStencil", DStencil);
}

void SkyboxPass::Execute() const
{
	Bind();
	CurrentGraphicsContext::Context()->DrawIndexed(Count, 0, 0);
}
