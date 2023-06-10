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

	UniquePtr<VertexShader> vertexShader = MakeUnique<VertexShader>("textureVS");
	UniquePtr<PixelShader>pixelShader = MakeUnique<PixelShader>("texturePS");

	AddShader(std::move(vertexShader));
	AddShader(std::move(pixelShader));

	struct VertexElementTex : public VertexElement
	{
		struct
		{
			float u;
			float v;
		} TexCoords;
	};

	auto data = Primitives::Cube::Create<VertexElementTex>();

	data.Vertices[0].TexCoords = { 0.0f,0.0f };
	data.Vertices[1].TexCoords = { 1.0f,0.0f };
	data.Vertices[2].TexCoords = { 0.0f,1.0f };
	data.Vertices[3].TexCoords = { 1.0f,1.0f };
	data.Vertices[4].TexCoords = { 0.0f,0.0f };
	data.Vertices[5].TexCoords = { 1.0f,0.0f };
	data.Vertices[6].TexCoords = { 0.0f,1.0f };
	data.Vertices[7].TexCoords = { 1.0f,1.0f };

	UniquePtr<VertexBuffer> vertexBuffer = MakeUnique<VertexBuffer>(data.Vertices, GetTypeShaders().GetBlob(ShaderType::Vertex));
	vertexBuffer->AddLayoutElement({ "Position", LayoutElement::DataType::Float3 })
				 .AddLayoutElement({ "TexCoord", LayoutElement::DataType::Float2 });

	AddBuffer(std::move(vertexBuffer));
	AddBuffer(MakeUnique<IndexBuffer>(data.Indices));
	//AddBuffer(MakeUnique< PSConstantBuffer<FaceColors>>(FaceColors{ {
	//		  { 1.0f,0.0f,1.0f, 1.0f },
	//		  { 1.0f,0.0f,0.0f, 1.0f },
	//		  { 0.0f,1.0f,0.0f, 1.0f },
	//		  { 0.0f,0.0f,1.0f, 1.0f },
	//		  { 1.0f,1.0f,0.0f, 1.0f },
	//		  { 0.0f,1.0f,1.0f, 1.0f },
	//	} }));

	const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetProjection();
	AddBuffer(MakeUnique<Uniform<DirectX::XMMATRIX>>(MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(projection), projection));
}

void Cube::Init()
{
	InitializeType();

	InstanceBuffers.Add(MakeUnique<Uniform<DirectX::XMMATRIX>>(
		MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(GetTransform(), 1),
		this->Transform.GetMatrix()));

	Components.Add(MakeUnique<Texture>("sample.jpg"));
}

inline void Cube::Update()
{
	Transform.Update();
}