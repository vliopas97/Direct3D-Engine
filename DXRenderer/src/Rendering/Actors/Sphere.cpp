#include "Sphere.h"

#include "Primitives.h"
#include "Rendering\Graphics.h"
#include "Rendering\CurrentGraphicsContext.h"

Sphere::Sphere()
{
	Init();
}

Sphere::Sphere(const TransformationIntrinsics& intrinsics)
	:Actor(intrinsics)
{
	Init();
}

void Sphere::Update()
{
	Transform.Update();
}

void Sphere::Init()
{
	using namespace DirectX;

	UniquePtr<VertexShader> vertexShader = MakeUnique<VertexShader>("defaultVS");
	UniquePtr<PixelShader>pixelShader = MakeUnique<PixelShader>("defaultPS");

	Actor::Add(MakeShared<VertexShader>("defaultVS"));
	Actor::Add(MakeShared<PixelShader>("defaultPS"));
	
	auto data = Primitives::Sphere::Create<Primitives::VertexElement>();
	data.Transform(XMMatrixScaling(0.1f, 0.1f, 0.1f));

	UniquePtr<VertexBuffer> vertexBuffer = MakeUnique<VertexBuffer>("Sphere", data.Vertices,
		BufferLayout{ {"Position", LayoutElement::DataType::Float3} },
		Shaders.GetBlob(ShaderType::VertexS));
	Add(std::move(vertexBuffer));
	Add(MakeUnique<IndexBuffer>("Sphere", data.Indices));

	const XMMATRIX& viewProjection = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetViewProjection();
	Add(MakeUnique<Uniform<XMMATRIX>>(MakeUnique<VSConstantBuffer<XMMATRIX>>("ViewProj", viewProjection),
											   viewProjection));

	Buffers.Add(MakeUnique<Uniform<XMMATRIX>>(MakeUnique<VSConstantBuffer<XMMATRIX>>("Transform", GetTransform(), 1),
											  this->Transform.GetMatrix()));

	Buffers.Add(MakeUnique<PSConstantBuffer<XMVECTOR>>("Color",XMVECTOR(XMVectorSet(1.0f, 0.9f, 0.6f, 1.0f))
	));
}