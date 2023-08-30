#include "CameraViewer.h"
#include "Rendering/CurrentGraphicsContext.h"

CameraViewer::CameraViewer()
{
	using namespace DirectX;

	const auto tag = "$cam";
	const float x = 1.0f;
	const float y = 0.75f;
	const float z = -2.0f;

	std::vector<DirectX::XMFLOAT3> vertices
	{ { -x, y, 0.0f },
	 { x, y, 0.0f },
	 { x, -y, 0.0f },
	 { -x, -y, 0.0f },
	 { 0.0f, 0.0f, z } };

	std::vector<unsigned short> indices = {
		0, 1, 1, 2, 2, 3, 3, 0, 0, 4,
		1, 4, 2, 4, 3, 4 };

	VertexBuffer vb{ tag, vertices, BufferLayout{ LayoutElement::ElementType::Position3 }, D3D11_PRIMITIVE_TOPOLOGY_LINELIST };
	IndexBuffer ib{ tag, indices };

	Technique standard(Channels::Main);
	{
		Step first("phong");

		VertexShader vs("default");
		first.Add<InputLayout>(tag, vb.GetLayout(), vs.GetBlob());
		first.Add<VertexShader>(vs);
		first.Add<PixelShader>("colorInput");
		first.Add<UniformVS<XMMATRIX>>(tag + UIDTag(), Transform.GetMatrix(), 1);
		first.Add<UniformVS<XMMATRIX>>(tag, CurrentGraphicsContext::GraphicsInfo->GetViewProjection());
		first.Add<PS<XMFLOAT4>>(tag + std::string("Color"), DirectX::XMFLOAT4{ 0.2f, 0.2f, 0.6f, 1.0f });
		first.Add<RasterizerState>(false);
		standard.PushBack(std::move(first));
	}

	Add<VertexBuffer>(vb);
	Add<IndexBuffer>(ib);
	Add(std::move(standard));
}