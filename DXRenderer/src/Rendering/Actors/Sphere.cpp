#include "Sphere.h"

#include "Primitives.h"
#include "Rendering\Graphics.h"
#include "Rendering\CurrentGraphicsContext.h"

Sphere::Sphere()
{
	Init();
}

Sphere::Sphere(const TransformationIntrinsics& intrinsics)
	:ActorBase(intrinsics)
{
	Init();
}

void Sphere::Update()
{
	Transform.Update();
}

void Sphere::InitializeType()
{
	namespace WRL = Microsoft::WRL;

	if (IsInitialized())
		return;

	UniquePtr<VertexShader> vertexShader = MakeUnique<VertexShader>("defaultVS");
	UniquePtr<PixelShader>pixelShader = MakeUnique<PixelShader>("defaultPS");

	Actor::Add(MakeShared<VertexShader>("defaultVS"));
	Actor::Add(MakeShared<PixelShader>("defaultPS"));
	
	auto data = Primitives::Sphere::Create<Primitives::VertexElement>();
	data.Transform(DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f));

	UniquePtr<VertexBuffer> vertexBuffer = MakeUnique<VertexBuffer>(data.Vertices,
		BufferLayout{ {"Position", LayoutElement::DataType::Float3} },
		Shaders.GetBlob(ShaderType::VertexS));
	Add(std::move(vertexBuffer));
	Add(MakeUnique<IndexBuffer>(data.Indices));

	const DirectX::XMMATRIX& viewProjection = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetViewProjection();
	Add(MakeUnique<Uniform<DirectX::XMMATRIX>>(MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(viewProjection), viewProjection));
}

void Sphere::Init()
{
	InitializeType();

	InstanceBuffers.Add(MakeUnique<Uniform<DirectX::XMMATRIX>>(
		MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(GetTransform(), 1),
		this->Transform.GetMatrix()));

	InstanceBuffers.Add(MakeUnique<PSConstantBuffer<DirectX::XMVECTOR>>(
		DirectX::XMVECTOR(DirectX::XMVectorSet(1.0f, 0.9f, 0.6f, 1.0f))
	));
}