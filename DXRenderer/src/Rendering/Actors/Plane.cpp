#include "Plane.h"

#include "Rendering/Material.h"

Plane::Plane()
{
	Init();
}

Plane::Plane(const TransformationIntrinsics& intrinsics)
	:Actor(intrinsics)
{
	Init();
}

inline void Plane::GUI()
{
	ImGui::Begin("Plane");

	float roll = DirectX::XMConvertToRadians(Roll);
	float pitch = DirectX::XMConvertToRadians(Pitch);
	float yaw = DirectX::XMConvertToRadians(Yaw);

	ImGui::NextColumn();
	ImGui::Text("Orientation");
	ImGui::SliderAngle("Roll", &roll, -180.0f, 180.0f);
	ImGui::SliderAngle("Pitch", &pitch, -180.0f, 180.0f);
	ImGui::SliderAngle("Yaw", &yaw, -180.0f, 180.0f);

	ImGui::Text("Position");
	ImGui::SliderFloat("X", &X, -20.0f, 20.0f);
	ImGui::SliderFloat("Y", &Y, -20.0f, 20.0f);
	ImGui::SliderFloat("Z", &Z, -20.0f, 20.0f);

	roll = DirectX::XMConvertToDegrees(roll);
	pitch = DirectX::XMConvertToDegrees(pitch);
	yaw = DirectX::XMConvertToDegrees(yaw);

	Roll = roll;
	Pitch = pitch;
	Yaw = yaw;
	ImGui::End();
}

void Plane::Init()
{
	using namespace DirectX;

	Add<VertexShader>("PhongLoadTexture");
	Add<PixelShader>("PhongNormal");

	struct VertexElementNormalTex : public Primitives::VertexElement
	{
		XMFLOAT3 Normal;
		XMFLOAT2 TexCoords;
	};

	auto data = Primitives::Plane::CreateWTextureCoords<VertexElementNormalTex>();

	UniquePtr<VertexBuffer> vertexBuffer = MakeUnique<VertexBuffer>("Cube",
																	data.Vertices,
																	BufferLayout{
																		{ "Position", LayoutElement::DataType::Float3 },
																		{ "Normal", LayoutElement::DataType::Float3 },
																		{ "TexCoords", LayoutElement::DataType::Float2 }
																	},
																	Shaders.GetBlob(ShaderType::VertexS));

	Add(std::move(vertexBuffer));
	Add<IndexBuffer>("Plane", data.Indices);

	const XMMATRIX& view = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetView();
	Add<UniformPS<XMMATRIX>>("View", view, 2);

	const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetProjection();
	Add<UniformVS<XMMATRIX>>("Proj", projection, 1);

	auto& modelView = ModelView;
	Add<UniformVS<XMMATRIX>>("Transform", modelView);

	auto material = MakeUnique<Material>(1);
	material->Properties.specularIntensity = 0.3f;
	material->Properties.Shininess = 20.0f;
	Add(std::move(material));
	Add<Texture>("Img\\brickwall.jpg", 0);
	Add<Texture>("Img\\brickwall_normal.jpg", 1);
}
