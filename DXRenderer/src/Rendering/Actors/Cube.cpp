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
	Add<VertexShader>("Phong");
	Add<PixelShader>("Phong");

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
	Add<IndexBuffer>("Cube", data.Indices);

	const XMMATRIX& view = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetView();
	Add<UniformVS<XMMATRIX>>("View", view);
	Add<UniformPS<XMMATRIX>>("View", view, 2);

	const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetProjection();
	Add<UniformVS<XMMATRIX>>("Proj", projection, 1);

	auto& transform = *reinterpret_cast<const XMMATRIX*>(&Transform.GetMatrix());
	Add<UniformVS<XMMATRIX>>("Transform", transform, 2);

	Add<Material>(1);
}