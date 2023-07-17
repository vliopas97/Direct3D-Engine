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
	:ActorBase(intrinsics)
{
	Init();
}

inline void Cube::InitializeType()
{	
	namespace WRL = Microsoft::WRL;

	if (IsInitialized())
		return;

	Actor::Add(MakeShared<VertexShader>("PhongVS"));
	Actor::Add(MakeShared<PixelShader>("PhongPS"));

	struct VertexElementNormal : public Primitives::VertexElement
	{
		DirectX::XMFLOAT3 Normal;
	};

	auto data = Primitives::Cube::CreateWNormals<VertexElementNormal>();

	UniquePtr<VertexBuffer> vertexBuffer = MakeUnique<VertexBuffer>(data.Vertices,
		BufferLayout{
			{ "Position", LayoutElement::DataType::Float3 },
			{ "Normal", LayoutElement::DataType::Float3 }
		},
		Shaders.GetBlob(ShaderType::VertexS));

	Add(std::move(vertexBuffer));
	Add(MakeUnique<IndexBuffer>(data.Indices));

	const DirectX::XMMATRIX& view = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetView();
	Add(MakeUnique<Uniform<DirectX::XMMATRIX>>(MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(view), view));
	Add(MakeUnique<Uniform<DirectX::XMMATRIX>>(MakeUnique<PSConstantBuffer<DirectX::XMMATRIX>>(view, 2), view));

	const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetProjection();
	Add(MakeUnique<Uniform<DirectX::XMMATRIX>>(MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(projection, 1), projection));

}

void Cube::Init()
{
	InitializeType();

	InstanceBuffers.Add(MakeUnique<Uniform<DirectX::XMMATRIX>>(
		MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(GetTransform(), 2),
		this->Transform.GetMatrix()));

	Components.Add(MakeUnique < Material>(1));
}