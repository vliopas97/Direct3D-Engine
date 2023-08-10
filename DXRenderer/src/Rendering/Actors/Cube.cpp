#include "Cube.h"

#include "Primitives.h"
#include "Rendering\Graphics.h"
#include "Rendering\CurrentGraphicsContext.h"
#include "Rendering\Material.h"
#include "Rendering\State.h"

Cube::Cube()
	:Outline(MakeUnique<CubeOutline>(*this))
{
	Init();
}

Cube::Cube(const TransformationIntrinsics& intrinsics)
	:Actor(intrinsics), Outline(MakeUnique<CubeOutline>(*this))
{
	Init();
}

void Cube::Draw()
{
	Actor::Draw();
	DrawOutline();
}

inline void Cube::Tick(float delta)
{
	Actor::Tick(delta);
	Outline->Tick(delta);
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
	Add<UniformPS<XMMATRIX>>("CubeView", view, 2);

	const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetProjection();
	Add<UniformVS<XMMATRIX>>("CubeProj", projection, 1);

	auto& modelView = ModelView;
	Add<UniformVS<XMMATRIX>>("CubeTransform", modelView);

	Add<Material>(1);

	Add<StencilState<DepthStencilMode::Write>>("CubeDepthStencil");
}

void Cube::DrawOutline()
{
	if (!IsOutlined) return;

	Outline->Draw();
}


CubeOutline::CubeOutline(const Cube& cube)
	:CubeRef(cube)
{
	using namespace DirectX;

	Add<VertexShader>("colorInput");
	Add<PixelShader>("colorInput");

	auto data = Primitives::Cube::Create<Primitives::VertexElement>();

	Add<VertexBuffer>("CubeOutline", data.Vertices,
					  BufferLayout{{ "Position", LayoutElement::DataType::Float3 }},
					  Shaders.GetBlob(ShaderType::VertexS));

	Add<IndexBuffer>("CubeOutline", data.Indices);

	Add<UniformVS<XMMATRIX>>("CubeOutlineTransform", ModelView);
	const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetProjection();
	Add<UniformVS<XMMATRIX>>("CubeOutlineProj", projection, 1);

	Add<PS<XMFLOAT4>>("CubeOutlineColor", Color);
	Add<StencilState<DepthStencilMode::Mask>>("cubeoutlinedepthstencil");
}

void CubeOutline::Tick(float delta)
{
	Transform.GetMatrix() = DirectX::XMMatrixScaling(1.03f, 1.03f, 1.03f) * CubeRef.GetTransform();
	ModelView = Transform.GetMatrix() * CurrentGraphicsContext::GraphicsInfo->GetCamera().GetView();
}
