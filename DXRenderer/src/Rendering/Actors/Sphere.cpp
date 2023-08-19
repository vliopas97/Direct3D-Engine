#include "Sphere.h"

#include "Primitives.h"
#include "Rendering\Graphics.h"
#include "Rendering\CurrentGraphicsContext.h"
#include "Rendering\State.h"
#include "Rendering/RenderGraph/RenderQueue.h"

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

	auto data = Primitives::Sphere::Create<Primitives::VertexElement>();
	data.Transform(XMMatrixScaling(0.1f, 0.1f, 0.1f));

	UniquePtr<VertexBuffer> vertexBuffer = MakeUnique<VertexBuffer>("Sphere", data.Vertices,
																	BufferLayout{ {"Position", LayoutElement::DataType::Float3} });

	Add<IndexBuffer>("Sphere", data.Indices);

	Technique standard;
	{
		Step first("phong");

		VertexShader vertexShader("default");
		first.Add<PixelShader>("default");
		first.Add<InputLayout>("Sphere", vertexBuffer->GetLayout(), vertexShader.GetBlob());
		first.Add<VertexShader>(vertexShader);

		const XMMATRIX& viewProjection = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetViewProjection();
		first.Add<UniformVS<XMMATRIX>>("SphereViewProj", viewProjection);

		auto& transform = *reinterpret_cast<const XMMATRIX*>(&Transform.GetMatrix());
		first.Add<UniformVS<XMMATRIX>>("SphereTransform", transform, 1);

		first.Add<PS<XMVECTOR>>("SphereColor", XMVECTOR(XMVectorSet(1.0f, 0.9f, 0.6f, 1.0f)));
		first.Add<RasterizerState>(false);
		standard.PushBack(std::move(first));
	}

	Add(std::move(vertexBuffer));
	Add(std::move(standard));
}