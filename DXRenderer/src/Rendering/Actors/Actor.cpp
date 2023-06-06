#include "Actor.h"

#include "Rendering\Graphics.h"
#include "Rendering\CurrentGraphicsContext.h"
#include "Rendering\Buffer.h"
#include "Rendering\Shader.h"

#include <vector>


TransformationMatrix::TransformationMatrix()
	: Intrinsics(), Matrix(DirectX::XMMatrixIdentity())
{
}

inline TransformationMatrix::TransformationMatrix(const DirectX::XMMATRIX& matrix)
	: Intrinsics(), Matrix(matrix)
{
}

inline TransformationMatrix::TransformationMatrix(const TransformationIntrinsics& intrinsics)
	: Intrinsics(intrinsics), Matrix(DirectX::XMMatrixIdentity())
{
	Update();
}

inline void TransformationMatrix::Update()
{
	Matrix = DirectX::XMMatrixScaling(Sx, Sy, Sz) *
		DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(Pitch),
											  DirectX::XMConvertToRadians(Yaw),
											  DirectX::XMConvertToRadians(Roll)) *
		DirectX::XMMatrixTranslation(X, Y, Z);
}

Actor::Actor()
	: Transform(), Buffers(MakeUnique<BufferGroup>()), Shaders(MakeUnique<ShaderGroup>())
{
}

Actor::Actor(const TransformationIntrinsics& intrinsics)
	: Transform(intrinsics), Buffers(MakeUnique<BufferGroup>()), Shaders(MakeUnique<ShaderGroup>())
{
}

void Actor::AddBuffer(UniquePtr<Buffer> buffer)
{
	Buffers->AddBuffer(std::move(buffer));
}

void Actor::AddShader(UniquePtr<Shader> shader)
{
	Shaders->AddShader(std::move(shader));
}

void Actor::Draw()
{
	Shaders->Bind();
	Buffers->Bind();
	CurrentGraphicsContext::Context()->DrawIndexed(Buffers->GetIndexBuffer()->GetCount(), 0, 0);
}

DirectX::XMMATRIX Actor::GetTransform() const
{
	return Transform;
}

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
	namespace WRL = Microsoft::WRL;

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

	UniquePtr<VertexBuffer> vertexBuffer = MakeUnique<VertexBuffer>(vertices, Shaders->GetBlob(ShaderType::Vertex));
	vertexBuffer->AddLayoutElement({ "Position", LayoutElement::DataType::Float3 }).
		AddLayoutElement({ "Color", LayoutElement::DataType::UChar4Norm });

	AddBuffer(std::move(vertexBuffer));
	AddBuffer(MakeUnique<IndexBuffer>(indices));

	Transform.Update();

	UniquePtr<Uniform<DirectX::XMMATRIX>> Model = MakeUnique<Uniform<DirectX::XMMATRIX>>(
		MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(GetTransform()),
		this->Transform.Matrix);

	AddBuffer(std::move(Model));

	AddBuffer(MakeUnique< PSConstantBuffer<FaceColors>>(FaceColors{ {
		{ 1.0f,0.0f,1.0f, 1.0f },
			  { 1.0f,0.0f,0.0f, 1.0f },
			  { 0.0f,1.0f,0.0f, 1.0f },
			  { 0.0f,0.0f,1.0f, 1.0f },
			  { 1.0f,1.0f,0.0f, 1.0f },
			  { 0.0f,1.0f,1.0f, 1.0f },
		} }));

	const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetProjection();
	AddBuffer(MakeUnique<Uniform<DirectX::XMMATRIX>>(MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(projection, 1), projection));
}
