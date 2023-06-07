#include "Cube.h"
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

	std::vector<VertexElement> vertices = {
		{ -1.0f, -1.0f, -1.0f, 0, 0, 255, 1 },
		{ 1.0f, -1.0f, -1.0f, 0, 255, 255, 1 },
		{ -1.0f,  1.0f, -1.0f, 255, 0, 0, 1 },
		{ 1.0f,  1.0f, -1.0f, 255, 0, 255, 1 },
		{ -1.0f, -1.0f,  1.0f, 0, 255, 255, 1 },
		{ 1.0f, -1.0f,  1.0f, 255, 0, 255, 1 },
		{ -1.0f,  1.0f,  1.0f, 0, 255, 255, 1 },
		{ 1.0f,  1.0f,  1.0f, 0, 255, 0, 1 }
	};
	std::vector<unsigned short> indices =
	{
		0,2,1, 2,3,1,
		1,3,5, 3,7,5,
		2,6,3, 3,6,7,
		4,5,7, 4,7,6,
		0,4,2, 2,4,6,
		0,1,4, 1,5,4
	};

	UniquePtr<VertexBuffer> vertexBuffer = MakeUnique<VertexBuffer>(vertices, GetTypeShaders().GetBlob(ShaderType::Vertex));
	vertexBuffer->AddLayoutElement({ "Position", LayoutElement::DataType::Float3 }).
		AddLayoutElement({ "Color", LayoutElement::DataType::UChar4Norm });

	AddBuffer(std::move(vertexBuffer));
	AddBuffer(MakeUnique<IndexBuffer>(indices));
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
		this->Transform.Matrix));
}

inline void Cube::Update()
{
	Transform.Update();
}