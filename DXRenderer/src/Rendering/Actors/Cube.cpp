#include "Cube.h"

#include "Primitives.h"
#include "Rendering\Graphics.h"
#include "Rendering\CurrentGraphicsContext.h"
#include "Rendering\Material.h"
#include "Rendering\State.h"
#include "Rendering/RenderGraph/RenderQueue.h"

Cube::Cube()
{
	Init();
}

Cube::Cube(const TransformationIntrinsics& intrinsics)
	:Actor(intrinsics)
{
	Init();
}

void Cube::Submit(size_t channelsIn)
{
	Actor::Submit(channelsIn);
}

inline void Cube::Tick(float delta)
{
	Actor::Tick(delta);
	ModelViewOutline = DirectX::XMMatrixScaling(1.03f, 1.03f, 1.03f) * ModelView;
}

void Cube::Init()
{
	using namespace DirectX;
	struct VertexNormal : Primitives::VertexElement { DirectX::XMFLOAT3 Normal; };
	auto data = Primitives::Cube::CreateWNormals<VertexNormal>();

	UniquePtr<VertexBuffer> vertexBuffer = MakeUnique<VertexBuffer>(
		"Cube",
		data.Vertices,
		BufferLayout
		{ { "Position", LayoutElement::DataType::Float3 }, { "Normal", LayoutElement::DataType::Float3 } }
	);
	Add<IndexBuffer>("Cube", data.Indices);

	Technique standard(Channels::Main);
	{
		Step first("phong");
		VertexShader vs("ShadowCubeTest");
		first.Add<PixelShader>("ShadowCubeTest");
		first.Add<InputLayout>("Cube", vertexBuffer->GetLayout(), vs.GetBlob());
		first.Add<VertexShader>(vs);

		first.Add<UniformVS<XMMATRIX>>("CubeTransform" + UIDTag(), Transform.GetMatrix());
		first.Add<UniformVS<XMMATRIX>>("Cube" + UIDTag(), ModelView, 1);
		const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetProjection();
		first.Add<UniformVS<XMMATRIX>>("Cube", projection, 2);
		first.Add<PS<XMFLOAT4>>("CubeColor", DirectX::XMFLOAT4{ 0.7f, 0.7f, 0.7f, 1.0f }, 1);
		standard.PushBack(std::move(first));
	}
	Technique outline(Channels::Main);
	{
		Step mask("outlineMask");
		VertexShader vs("colorInput");
		mask.Add<InputLayout>("Cube1", vertexBuffer->GetLayout(), vs.GetBlob());

		mask.Add<UniformVS<XMMATRIX>>("Cube" + UIDTag(), ModelView);
		const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetProjection();
		mask.Add<UniformVS<XMMATRIX>>("Cube", projection, 1);
		outline.PushBack(std::move(mask));
	}
	{
		Step draw("outlineDraw");
		VertexShader vs("colorInput");
		draw.Add<InputLayout>("Cube2", vertexBuffer->GetLayout(), vs.GetBlob());

		draw.Add<UniformVS<XMMATRIX>>("Cube2" + UIDTag(), ModelViewOutline);
		const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetProjection();
		draw.Add<UniformVS<XMMATRIX>>("Cube", projection, 1);

		draw.Add<PS<XMFLOAT4>>("CubeColorOutline", DirectX::XMFLOAT4{ 1.0f, 0.4f, 0.4f, 1.0f });
		outline.PushBack(std::move(draw));
	}
	Technique shadowMap(Channels::Shadow);
	{
		Step draw("shadowMap");
		
		VertexShader vs("colorInput");
		draw.Add<InputLayout>("Cube3", vertexBuffer->GetLayout(), vs.GetBlob());
		draw.Add<UniformVS<XMMATRIX>>("Cube" + UIDTag(), Transform.GetMatrix());
		
		shadowMap.PushBack(std::move(draw));
	}

	Add(std::move(standard));
	Add(std::move(outline));
	Add(std::move(shadowMap));
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
	const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetProjection();
	Add<UniformVS<XMMATRIX>>("CubeOutlineProj", projection, 1);

	Add<PS<XMFLOAT4>>("CubeOutlineColor", Color);
	Add<StencilState<DepthStencilMode::Mask>>();
}

void CubeOutline::Tick(float delta)
{
	Transform.GetMatrix() = DirectX::XMMatrixScaling(1.03f, 1.03f, 1.03f) * CubeRef.GetTransform();
	ModelView = Transform.GetMatrix() * CurrentGraphicsContext::GraphicsInfo->GetView();
}