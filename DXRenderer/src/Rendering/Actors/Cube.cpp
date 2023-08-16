#include "Cube.h"

#include "Primitives.h"
#include "Rendering\Graphics.h"
#include "Rendering\CurrentGraphicsContext.h"
#include "Rendering\Material.h"
#include "Rendering\State.h"

Cube::Cube()
{
	Init();
}

Cube::Cube(const TransformationIntrinsics& intrinsics)
	:Actor(intrinsics)
{
	Init();
}

void Cube::Draw()
{
	Actor::Draw();
}

inline void Cube::Tick(float delta)
{
	Actor::Tick(delta);
	ModelViewOutline = DirectX::XMMatrixScaling(1.03f, 1.03f, 1.03f) * ModelView;
}

void Cube::Init()
{
	using namespace DirectX;

	auto data = Primitives::Cube::Create<Primitives::VertexElement>();

	UniquePtr<VertexBuffer> vertexBuffer = MakeUnique<VertexBuffer>(
		"Cube",
		data.Vertices,
		BufferLayout
		{ { "Position", LayoutElement::DataType::Float3 } }
	);
	Add<IndexBuffer>("Cube", data.Indices);


	Technique standard;
	{
		Step first(0);
		VertexShader vs("colorInput");
		first.Add<PixelShader>("colorInput");
		first.Add<InputLayout>("Cube", vertexBuffer->GetLayout(), vs.GetBlob());
		first.Add<VertexShader>(vs);

		first.Add<UniformVS<XMMATRIX>>("Cube", ModelView);
		const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetProjection();
		first.Add<UniformVS<XMMATRIX>>("Cube", projection, 1);

		first.Add<PS<XMFLOAT4>>("CubeColor", DirectX::XMFLOAT4{ 0.8f, 0.8f, 0.8f, 1.0f });
		standard.PushBack(std::move(first));
	}
	Technique outline;
	{
		Step mask(1);
		VertexShader vs("colorInput");
		mask.Add<InputLayout>("Cube1", vertexBuffer->GetLayout(), vs.GetBlob());
		mask.Add<VertexShader>(vs);

		mask.Add<UniformVS<XMMATRIX>>("Cube1", ModelView);
		const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetProjection();
		mask.Add<UniformVS<XMMATRIX>>("Cube1", projection, 1);
		outline.PushBack(std::move(mask));
	}
	{
		Step draw(2);
		VertexShader vs("colorInput");
		draw.Add<PixelShader>("colorInput");
		draw.Add<InputLayout>("Cube2", vertexBuffer->GetLayout(), vs.GetBlob());
		draw.Add<VertexShader>(vs);

		draw.Add<UniformVS<XMMATRIX>>("Cube2", ModelView);
		const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetProjection();
		draw.Add<UniformVS<XMMATRIX>>("Cube2", projection, 1);

		draw.Add<PS<XMFLOAT4>>("CubeColor2", DirectX::XMFLOAT4{ 1.0f, 0.4f, 0.4f, 1.0f });
		outline.PushBack(std::move(draw));
	}

	Add(std::move(standard));
	Add(std::move(outline));
	Add(std::move(vertexBuffer));
}

CubeOutline::CubeOutline(const Cube& cube)
	:CubeRef(cube)
{
	using namespace DirectX;

	Add<VertexShader>("colorInput");
	Add<PixelShader>("colorInput");

	auto data = Primitives::Cube::Create<Primitives::VertexElement>();

	auto vertexBuffer = MakeShared<VertexBuffer>("CubeOutline", data.Vertices,
												 BufferLayout{ { "Position", LayoutElement::DataType::Float3 } });
	Add<InputLayout>("Cube", vertexBuffer->GetLayout(), Shaders.GetBlob(ShaderType::VertexS));
	Add(std::move(vertexBuffer));
	Add<IndexBuffer>("CubeOutline", data.Indices);

	Add<UniformVS<XMMATRIX>>("CubeOutlineTransform", ModelView);
	const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetProjection();
	Add<UniformVS<XMMATRIX>>("CubeOutlineProj", projection, 1);

	Add<PS<XMFLOAT4>>("CubeOutlineColor", Color);
	Add<StencilState<DepthStencilMode::Mask>>();
}

void CubeOutline::Tick(float delta)
{
	Transform.GetMatrix() = DirectX::XMMatrixScaling(1.03f, 1.03f, 1.03f) * CubeRef.GetTransform();
	ModelView = Transform.GetMatrix() * CurrentGraphicsContext::GraphicsInfo->GetCamera().GetView();
}