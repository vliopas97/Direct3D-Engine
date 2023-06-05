#include "Actor.h"

#include "Rendering\CurrentGraphicsContext.h"
#include <vector>

inline void Actor::AddBuffer(UniquePtr<Buffer> buffer)
{
	Buffers.AddBuffer(std::move(buffer));
}

inline void Actor::AddShader(UniquePtr<Shader> shader)
{
	Shaders.AddShader(std::move(shader));
}

void Actor::Draw()
{
	Shaders.Bind();
	Buffers.Bind();
	CurrentGraphicsContext::Context()->DrawIndexed(Buffers.GetIndexBuffer()->GetCount(), 0, 0);
}

Cube::Cube()
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

	UniquePtr<VertexBuffer> vertexBuffer = MakeUnique<VertexBuffer>(vertices, Shaders.GetBlob(ShaderType::Vertex));
	vertexBuffer->AddLayoutElement({ "Position", LayoutElement::DataType::Float3 }).
		AddLayoutElement({ "Color", LayoutElement::DataType::UChar4Norm });

	AddBuffer(std::move(vertexBuffer));
	AddBuffer(MakeUnique<IndexBuffer>(indices));

	float angle = 40.0f;
	UniquePtr<VSConstantBuffer<DirectX::XMMATRIX>> model
		= MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(angle))
														  * DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(angle))
														  * DirectX::XMMatrixTranslation(0, 0, 4)
														  * DirectX::XMMatrixPerspectiveFovLH(5.0f, 16.0f / 9.0f, 0.1f, 10.0f));
	AddBuffer(std::move(model));

	AddBuffer(MakeUnique< PSConstantBuffer<FaceColors>>(FaceColors{ {
		{ 1.0f,0.0f,1.0f, 1.0f },
			  { 1.0f,0.0f,0.0f, 1.0f },
			  { 0.0f,1.0f,0.0f, 1.0f },
			  { 0.0f,0.0f,1.0f, 1.0f },
			  { 1.0f,1.0f,0.0f, 1.0f },
			  { 0.0f,1.0f,1.0f, 1.0f },
		} }));
}
