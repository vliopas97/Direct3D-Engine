#include "Sphere.h"

#include "Primitives.h"
#include "Rendering\Graphics.h"
#include "Rendering\CurrentGraphicsContext.h"
#include "Rendering\State.h"

Sphere::Sphere()
{
	Init();
}

Sphere::Sphere(const TransformationIntrinsics& intrinsics)
	:Actor(intrinsics)
{
	Init();
}

void Sphere::Tick(float delta)
{
	Transform.Update();
}

void Sphere::Init()
{
	using namespace DirectX;

	Add<VertexShader>("default");
	Add<PixelShader>("default");
	
	auto data = Primitives::Sphere::Create<Primitives::VertexElement>();
	data.Transform(XMMatrixScaling(0.1f, 0.1f, 0.1f));

	UniquePtr<VertexBuffer> vertexBuffer = MakeUnique<VertexBuffer>("Sphere", data.Vertices,
		BufferLayout{ {"Position", LayoutElement::DataType::Float3} });

	Add<InputLayout>("Sphere", vertexBuffer->GetLayout(), Shaders.GetBlob(ShaderType::VertexS));
	
	Add(std::move(vertexBuffer));
	Add<IndexBuffer>("Sphere", data.Indices);

	const XMMATRIX& viewProjection = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetViewProjection();
	Add<UniformVS<XMMATRIX>>("SphereViewProj", viewProjection);

	auto& transform = *reinterpret_cast<const XMMATRIX*>(&Transform.GetMatrix());
	Add<UniformVS<XMMATRIX>>("SphereTransform", transform, 1);

	Add<PS<XMVECTOR>>("SphereColor",XMVECTOR(XMVectorSet(1.0f, 0.9f, 0.6f, 1.0f)));

	Add<StencilState<>>("SphereDepthStencil");
}