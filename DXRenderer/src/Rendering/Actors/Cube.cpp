#include "Cube.h"

#include "Primitives.h"
#include "Rendering\Graphics.h"
#include "Rendering\CurrentGraphicsContext.h"
#include "Rendering\Material.h"

Cube::Cube()
{
	Init();
}

Cube::Cube(const TransformationIntrinsics& intrinsics)
	:Actor(intrinsics)
{
	Init();
}

void Cube::Init()
{
	using namespace DirectX;
	Add(MakeShared<VertexShader>("PhongVS"));
	Add(MakeShared<PixelShader>("PhongPS"));

	struct VertexElementNormal : public Primitives::VertexElement
	{
		XMFLOAT3 Normal;
	};

	auto data = Primitives::Cube::CreateWNormals<VertexElementNormal>();

	UniquePtr<VertexBuffer> vertexBuffer = MakeUnique<VertexBuffer>("Cube",
																	data.Vertices,
																	BufferLayout{
																		{ "Position", LayoutElement::DataType::Float3 },
																		{ "Normal", LayoutElement::DataType::Float3 }
																	},
																	Shaders.GetBlob(ShaderType::VertexS));

	Add(std::move(vertexBuffer));
	Add(MakeUnique<IndexBuffer>("Cube", data.Indices));

	const XMMATRIX& view = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetView();
	Add(MakeUnique<Uniform<XMMATRIX>>(MakeUnique<VSConstantBuffer<XMMATRIX>>("View", view), view));
	Add(MakeUnique<Uniform<XMMATRIX>>(MakeUnique<PSConstantBuffer<XMMATRIX>>("View", view, 2), view));

	const XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetProjection();
	Add(MakeUnique<Uniform<XMMATRIX>>(MakeUnique<VSConstantBuffer<XMMATRIX>>("Projection", projection, 1),
									  projection));

	Buffers.Add(MakeUnique<Uniform<XMMATRIX>>(MakeUnique<VSConstantBuffer<XMMATRIX>>("Transform", GetTransform(), 2),
											  this->Transform.GetMatrix()));

	Components.Add(MakeUnique < Material>(1));
}