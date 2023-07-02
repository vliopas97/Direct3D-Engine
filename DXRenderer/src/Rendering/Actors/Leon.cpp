#include "Leon.h"
#include "Rendering/CurrentGraphicsContext.h"
#include "Rendering\Material.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>
#include <imgui.h>


Leon::Leon()
{
	Init();
}

Leon::Leon(const TransformationIntrinsics& intrinsics)
	:ActorBase(intrinsics)
{
	Init();
}

void Leon::Draw()
{
	Root->Draw();
}

void Leon::Update()
{
	ActorBase<Leon>::Update();
	Root->SetTransform(Transform.GetMatrix());
	Root->Update();
}

void Leon::GUI()
{
	if (ImGui::Begin("Leon"))
	{
		ImGui::Columns(2, nullptr, true);

		Root->ShowTree();

	}
	ImGui::End();
}

void Leon::InitializeType()
{
	namespace WRL = Microsoft::WRL;

	if (IsInitialized())
		return;

	UniquePtr<VertexShader> vertexShader = MakeUnique<VertexShader>("PhongVS");
	UniquePtr<PixelShader>pixelShader = MakeUnique<PixelShader>("PhongPS");

	AddShader(std::move(vertexShader));
	AddShader(std::move(pixelShader));

	std::filesystem::path solutionPath = std::filesystem::current_path().parent_path();

	Assimp::Importer importer;
	auto model = importer.ReadFile(solutionPath.string() + "\\Content\\Model\\LeonKennedy.obj",
		aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

	VertexBufferBuilder builder{
		BufferLayout{
			{ LayoutElement::ElementType::Position3 },
			{ LayoutElement::ElementType::Normal }
		},
		GetTypeShaders().GetBlob(ShaderType::VertexS)
	};

	std::vector<unsigned short> indices;
	indices.reserve(model->mMeshes[0]->mNumFaces * 3 * model->mNumMeshes);

	unsigned int offset = 0;
	for (unsigned int j = 0; j < model->mNumMeshes; j++)
	{
		auto& mesh = model->mMeshes[j];
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			builder.EmplaceBack(DirectX::XMFLOAT3{ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z },
				* reinterpret_cast<DirectX::XMFLOAT3*>(&mesh->mNormals[i]));
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			const auto& face = mesh->mFaces[i];
			assert(face.mNumIndices == 3);
			indices.push_back(face.mIndices[0] + offset);
			indices.push_back(face.mIndices[1] + offset);
			indices.push_back(face.mIndices[2] + offset);
		}

		offset += mesh->mNumVertices;
	}

	AddBuffer(builder.Release());

	AddBuffer(MakeUnique<IndexBuffer>(indices));

	const DirectX::XMMATRIX& view = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetView();
	AddBuffer(MakeUnique<Uniform<DirectX::XMMATRIX>>(MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(view), view));
	AddBuffer(MakeUnique<Uniform<DirectX::XMMATRIX>>(MakeUnique<PSConstantBuffer<DirectX::XMMATRIX>>(view, 2), view));

	const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetProjection();
	AddBuffer(MakeUnique<Uniform<DirectX::XMMATRIX>>(MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(projection, 1), projection));
}

void Leon::Init()
{
	//InitializeType();

	//InstanceBuffers.Add(MakeUnique<Uniform<DirectX::XMMATRIX>>(
	//	MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(GetTransform(), 2),
	//	this->Transform.GetMatrix()));

	Components.Add(MakeUnique<Material>(1));

	Root = Node::Build("LeonKennedy.obj");
}