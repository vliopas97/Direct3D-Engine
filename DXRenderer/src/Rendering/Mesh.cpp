#include "Mesh.h"

inline PrimitiveComponent::PrimitiveComponent()
	:Buffers{}, Shaders{}
{
	DirectX::XMStoreFloat4x4(&Transform, DirectX::XMMatrixIdentity());
}

inline void PrimitiveComponent::SetTransform(DirectX::XMMATRIX transform)
{
	DirectX::XMStoreFloat4x4(&Transform, transform);
}

inline DirectX::XMMATRIX PrimitiveComponent::GetTransform() const
{
	return DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&Transform));
}

inline void PrimitiveComponent::Add(UniquePtr<Shader> shader)
{
	Shaders.Add(std::move(shader));
}

inline void PrimitiveComponent::Add(UniquePtr<Buffer> buffer)
{
	Buffers.Add(std::move(buffer));
}

inline Mesh::Mesh(const aiMesh& mesh)
{
	UniquePtr<VertexShader> vertexShader = MakeUnique<VertexShader>("PhongVS");
	UniquePtr<PixelShader>pixelShader = MakeUnique<PixelShader>("PhongPS");

	VertexBufferBuilder builder{
		BufferLayout{
			{ LayoutElement::ElementType::Position3 },
		{ LayoutElement::ElementType::Normal }
	},
		vertexShader->GetBlob()
	};

	std::vector<unsigned short> indices;
	indices.reserve(mesh.mNumFaces * 3);

	for (unsigned int i = 0; i < mesh.mNumVertices; i++)
	{
		builder.EmplaceBack(DirectX::XMFLOAT3{ mesh.mVertices[i].x, mesh.mVertices[i].y, mesh.mVertices[i].z },
			* reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mNormals[i]));
	}

	for (unsigned int i = 0; i < mesh.mNumFaces; i++)
	{
		const auto& face = mesh.mFaces[i];
		assert(face.mNumIndices == 3);
		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
	}

	Add(std::move(vertexShader));
	Add(std::move(pixelShader));

	Add(builder.Release());
	Add(MakeUnique<IndexBuffer>(indices));

	const DirectX::XMMATRIX& view = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetView();
	Add(MakeUnique<Uniform<DirectX::XMMATRIX>>(MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(view), view));
	Add(MakeUnique<Uniform<DirectX::XMMATRIX>>(MakeUnique<PSConstantBuffer<DirectX::XMMATRIX>>(view, 2), view));

	const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetProjection();
	Add(MakeUnique<Uniform<DirectX::XMMATRIX>>(MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(projection, 1), projection));

	Add(MakeUnique<Uniform<DirectX::XMMATRIX>>(
		MakeUnique<VSConstantBuffer<DirectX::XMMATRIX>>(GetTransform(), 2),
		*reinterpret_cast<const DirectX::XMMATRIX*>(&Transform)));
}

inline void Mesh::Bind() const
{
	Shaders.Bind();
	Buffers.Bind();
}

inline void Mesh::Draw()
{
	Bind();
	const IndexBuffer* ptr = Buffers.GetIndexBuffer();
	ASSERT(ptr);
	CurrentGraphicsContext::Context()->DrawIndexed(ptr->GetCount(), 0, 0);
}

Node::Node() 
	:Children{}, Meshes{}
{
	DirectX::XMStoreFloat4x4(&Transform, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&RelativeTransform, DirectX::XMMatrixIdentity());
}

void Node::SetTransform(DirectX::XMMATRIX transform)
{
	DirectX::XMStoreFloat4x4(&Transform, transform);
}

DirectX::XMMATRIX Node::GetTransform() const
{
	return DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&Transform));
}

void Node::SetRelativeTransform(DirectX::XMMATRIX transform)
{
	auto newTransform = DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&Transform),
		DirectX::XMLoadFloat4x4(&RelativeTransform));
}

DirectX::XMMATRIX Node::GetRelativeTransform() const
{
	return DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&RelativeTransform));
}

inline void Node::SetupAttachment(Node* parent)
{
	if (parent && parent != Parent && parent != this)
	{
		auto* oldParent = Parent;
		Parent = parent;

		auto it = std::find_if(oldParent->Children.begin(), oldParent->Children.end(), [this](auto& uniquePtr)
			{
				return uniquePtr.get() == this;
			});

		ASSERT(it != oldParent->Children.end());
		Parent->Children.emplace_back(std::unique_ptr<Node>(it->release()));
		oldParent->Children.erase(it);
	}
}

void Node::Update()
{
	for (auto* mesh : Meshes)
	{
		mesh->SetTransform(DirectX::XMLoadFloat4x4(&Transform));
	}

	for (auto& child : Children)
	{
		child->SetTransform(DirectX::XMLoadFloat4x4(&Transform) * child->GetRelativeTransform());
		child->Update();
	}
}

void Node::Draw()
{
	for (auto* mesh : Meshes)
		mesh->Draw();

	for (auto& child : Children)
		child->Draw();
}

UniquePtr<Node> Node::Build(const std::string& filename)
{
	Assimp::Importer imp;
	std::filesystem::path solutionPath = std::filesystem::current_path().parent_path();

	const auto scene = imp.ReadFile(solutionPath.string() + "\\Content\\Model\\" + filename,
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices
	);

	return BuildImpl(*scene, *scene->mRootNode);
}

void Node::SetupChild(UniquePtr<Node> child)
{
	ASSERT(child);
	Children.emplace_back(std::move(child));
}

inline UniquePtr<Node> Node::BuildImpl(const aiScene& scene, const aiNode& node)
{
	const auto relativeTransform = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&node.mTransformation));
	std::vector<Mesh*> meshes;

	for (size_t i = 0; i < node.mNumMeshes; i++)
	{
		const auto index = node.mMeshes[i];
		meshes.emplace_back(new Mesh(*scene.mMeshes[index]));
	}

	UniquePtr<Node> customNode = MakeUnique<Node>();
	customNode->SetRelativeTransform(relativeTransform);
	customNode->Meshes = std::move(meshes);

	for (size_t i = 0; i < node.mNumChildren; i++)
	{
		customNode->SetupChild(BuildImpl(scene, *node.mChildren[i]));
	}

	return std::move(customNode);
}
