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

	UniquePtr<VertexShader> vertexShader = MakeUnique<VertexShader>("PhongVS");
	UniquePtr<PixelShader>pixelShader = MakeUnique<PixelShader>("PhongPS");

	AddShader(std::move(vertexShader));
	AddShader(std::move(pixelShader));

	struct VertexElementNormal : public VertexElement
	{
		DirectX::XMFLOAT3 Normal;
	};

	auto data = Primitives::Cube::CreateWNormals<VertexElementNormal>();

	UniquePtr<VertexBuffer> vertexBuffer = MakeUnique<VertexBuffer>(data.Vertices, GetTypeShaders().GetBlob(ShaderType::Vertex));
	vertexBuffer->AddLayoutElement({ "Position", LayoutElement::DataType::Float3 })
				 .AddLayoutElement({ "Normal", LayoutElement::DataType::Float3 });

	AddBuffer(std::move(vertexBuffer));
	AddBuffer(MakeUnique<IndexBuffer>(data.Indices));

	const DirectX::XMMATRIX& view = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetView();
	AddBuffer(MakeUnique<Uniform<DirectX::XMMATRIX>>(MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(view), view));
	AddBuffer(MakeUnique<Uniform<DirectX::XMMATRIX>>(MakeUnique<PSConstantBuffer<DirectX::XMMATRIX>>(view, 2), view));

	const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetProjection();
	AddBuffer(MakeUnique<Uniform<DirectX::XMMATRIX>>(MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(projection, 1), projection));


}

void Cube::Init()
{
	InitializeType();

	InstanceBuffers.Add(MakeUnique<Uniform<DirectX::XMMATRIX>>(
		MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(GetTransform(), 2),
		this->Transform.GetMatrix()));

	Components.Add(MakeUnique < Material>(1));
}

inline void Cube::Update()
{
	Transform.Update();
}