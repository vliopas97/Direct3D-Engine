#include "Cube.h"

#include "Primitives.h"
#include "Rendering\Graphics.h"
#include "Rendering\CurrentGraphicsContext.h"

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

	Transform.Update();

	UniquePtr<VertexShader> vertexShader = MakeUnique<VertexShader>("VertexShader");
	UniquePtr<PixelShader>pixelShader = MakeUnique<PixelShader>("PixelShader");

	AddShader(std::move(vertexShader));
	AddShader(std::move(pixelShader));

	auto data = Primitives::Cube::Create<VertexElement>();

	UniquePtr<VertexBuffer> vertexBuffer = MakeUnique<VertexBuffer>(data.Vertices, GetTypeShaders().GetBlob(ShaderType::Vertex));
	vertexBuffer->AddLayoutElement({ "Position", LayoutElement::DataType::Float3 });

	AddBuffer(std::move(vertexBuffer));
	AddBuffer(MakeUnique<IndexBuffer>(data.Indices));
	AddBuffer(MakeUnique< PSConstantBuffer<FaceColors>>(FaceColors{ {
			  { 1.0f,0.0f,1.0f, 1.0f },
			  { 1.0f,0.0f,0.0f, 1.0f },
			  { 0.0f,1.0f,0.0f, 1.0f },
			  { 0.0f,0.0f,1.0f, 1.0f },
			  { 1.0f,1.0f,0.0f, 1.0f },
			  { 0.0f,1.0f,1.0f, 1.0f },
		} }));

	const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetProjection();
	AddBuffer(MakeUnique<Uniform<DirectX::XMMATRIX>>(MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(projection), projection));
}

void Cube::Init()
{
	InitializeType();

	InstanceBuffers.Add(MakeUnique<Uniform<DirectX::XMMATRIX>>(
		MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(GetTransform(), 1),
		this->Transform.GetMatrix()));
}

inline void Cube::Update()
{
	Transform.Update();
}