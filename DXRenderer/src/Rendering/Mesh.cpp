#include "Mesh.h"
#include "Utilities.h"

#include "Actors/Model.h"
#include "Rendering/Material.h"
#include "Rendering/State.h"

class NodeInternal : public NodeBase
{
public:
	NodeInternal(Model& actor, const std::string& name = "Unknown")
		:NodeBase(actor, name)
	{
		DirectX::XMStoreFloat4x4(&Transform, DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&RelativeTransform, DirectX::XMMatrixIdentity());
	}

	void SetTransform(DirectX::XMMATRIX transform) override
	{
		DirectX::XMStoreFloat4x4(&Transform, transform);
	}

	DirectX::XMMATRIX GetTransform() const override
	{
		return DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&Transform));
	}

	virtual void SetRelativeTransform(DirectX::XMMATRIX transform)
	{
		DirectX::XMStoreFloat4x4(&RelativeTransform, transform);
	}

	DirectX::XMMATRIX GetRelativeTransform() const
	{
		return DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&RelativeTransform));
	}

	void Update()
	{
		for (auto* mesh : Meshes)
		{
			mesh->SetTransform(GetTransform());
		}

		for (auto& child : Children)
		{
			child->SetTransform(GetTransform() * child->GetRelativeTransform());
			child->Update();
		}
	}

	void Draw()
	{
		for (auto* mesh : Meshes)
			mesh->Draw();

		for (auto& child : Children)
			child->Draw();
	}

	void SetupChild(UniquePtr<NodeInternal> child)
	{
		ASSERT(child);
		child->Parent = this;
		Children.emplace_back(std::move(child));
	}

	void GUITransform() override
	{
		glm::mat transform = *reinterpret_cast<const glm::mat4x4*>(&RelativeTransform);

		glm::vec3 translate(transform[3]);
		translate.x *= -1;
		translate.y *= -1;

		transform = glm::transpose(transform);
		glm::quat quaternion = glm::quat_cast(transform);
		glm::vec3 angles = glm::eulerAngles(quaternion);
		angles.z *= -1;

		// Convert the quaternion to Euler angles
		float& roll = angles.z;
		float& pitch = angles.x;
		float& yaw = angles.y;

		float& x = translate.x;
		float& y = translate.y;
		float& z = translate.z;

		ImGui::NextColumn();
		ImGui::Text("Orientation (Relative)");
		ImGui::SliderAngle("Roll", &roll, -180.0f, 180.0f);
		ImGui::SliderAngle("Pitch", &pitch, -180.0f, 180.0f);
		ImGui::SliderAngle("Yaw", &yaw, -180.0f, 180.0f);

		ImGui::Text("Position (Relative)");
		ImGui::SliderFloat("X", &x, -20.0f, 20.0f);
		ImGui::SliderFloat("Y", &y, -20.0f, 20.0f);
		ImGui::SliderFloat("Z", &z, -20.0f, 20.0f);

		DirectX::XMMATRIX newTransform =
			DirectX::XMMatrixRotationRollPitchYaw(-pitch, -yaw, roll) * DirectX::XMMatrixTranslation(-x, -y, z);
		SetRelativeTransform(newTransform);
	}

private:
	const NodeBase* Parent = nullptr;

	DirectX::XMFLOAT4X4 Transform;
	DirectX::XMFLOAT4X4 RelativeTransform;

	friend class Node;
};

inline PrimitiveComponent::PrimitiveComponent()
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

inline Mesh::Mesh(const aiMesh& mesh, const std::string& meshName)
	:Name(meshName)
{
	Init(mesh);
}

Mesh::Mesh(const aiMesh& mesh, const std::string& meshName, const aiMaterial* const* materials, const std::string& path)
	:Name(meshName)
{
	LoadMaterial(mesh, materials, path);
	Init(mesh);
}

inline void Mesh::Bind() const
{
	Shaders.Bind();
	Buffers.Bind();
	Components.Bind();
}

inline void Mesh::Draw()
{
	Bind();
	const IndexBuffer* ptr = Buffers.GetIndexBuffer();
	ASSERT(ptr);
	CurrentGraphicsContext::Context()->DrawIndexed(ptr->GetCount(), 0, 0);
}

void Mesh::Init(const aiMesh& mesh)
{
	using namespace DirectX;

	auto [vertexName, pixelName] = ResolveShaders();
	Add<VertexShader>(vertexName);
	Add<PixelShader>(pixelName);

	ResolveVertexIndexBuffers(mesh);

	const DirectX::XMMATRIX& view = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetView();
	Add<UniformVS<XMMATRIX>>(Name + "View", view);
	Add<UniformPS<XMMATRIX>>(Name + "View", view, 2);

	const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetProjection();
	Add<UniformVS<XMMATRIX>>(Name + "Proj", projection, 1);

	auto& transform = *reinterpret_cast<const XMMATRIX*>(&Transform);
	Add<UniformVS<XMMATRIX>>(Name + "Transform", transform, 2);

	if (!HasSpecular)
	{
		auto material = MakeUnique<Material>(1);
		material->Properties.Shininess = Shininess;
		Add(std::move(material));
	}

	Add<RasterizerState>(HasAlphaDiffuse);
}

void Mesh::LoadMaterial(const aiMesh& mesh, const aiMaterial* const* materials, const std::string& path)
{
	ASSERT(materials);

	DirectX::XMFLOAT4 diffuseColor = { 0.45f,0.45f,0.85f,1.0f };

	HasMaterial = mesh.mMaterialIndex >= 0;
	if (!HasMaterial)
		return;

	auto& material = materials[mesh.mMaterialIndex];
	aiString filename;
	if (material->GetTexture(aiTextureType_DIFFUSE, 0, &filename) == aiReturn_SUCCESS)
	{
		HasDiffuse = true;
		auto t = MakeUnique<Texture>(path + filename.C_Str());
		HasAlphaDiffuse = t->HasAlpha();
		Add(std::move(t));
	}
	else
		material->Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor3D&>(diffuseColor));

	if (material->GetTexture(aiTextureType_NORMALS, 0, &filename) == aiReturn_SUCCESS)
	{
		HasNormals = true;
		Add<Texture>(path + filename.C_Str(), 1);
	}

	if (material->GetTexture(aiTextureType_SPECULAR, 0, &filename) == aiReturn_SUCCESS)
	{
		HasSpecular = true;
		Add<Texture>(path + filename.C_Str(), 2);
	}
	else
		material->Get(AI_MATKEY_SHININESS, Shininess);
}

std::pair<const char*, const char*> Mesh::ResolveShaders() const
{
	const char* vertexName, * pixelName;

	// Vertex Shader resolution
	if (HasDiffuse && HasNormals)
		vertexName = "PhongNormalLoadTexture";
	else if (HasDiffuse && !HasNormals)
		vertexName = "PhongLoadTexture";
	else
		vertexName = "Phong";

	// Pixel Shader resolution
	if (HasDiffuse && HasNormals && HasSpecular)
		pixelName = HasAlphaDiffuse ? "PhongNormalLoadTextureWSpecularWTransparency" : "PhongNormalLoadTextureWSpecular";
	else if (HasDiffuse && HasNormals && !HasSpecular)
		pixelName = "PhongNormalLoadTexture";
	else if (HasDiffuse && !HasNormals && HasSpecular)
		pixelName = "PhongLoadTextureWSpecular";
	else if (HasDiffuse && !HasNormals && !HasSpecular)
		pixelName = "PhongLoadTexture";
	else
		pixelName = "Phong";

	return { vertexName, pixelName };
}

void Mesh::ResolveVertexIndexBuffers(const aiMesh& mesh)
{
	if (HasDiffuse && HasNormals)
	{
		BufferLayout layout{
		{ LayoutElement::ElementType::Position3 },
		{ LayoutElement::ElementType::Normal },
		{ LayoutElement::ElementType::Tangent},
		{ LayoutElement::ElementType::Bitangent},
		{ LayoutElement::ElementType::TexCoords}
		};

		VertexBufferBuilder builder{ Name + "VertexBufferModel", std::move(layout), Shaders.GetBlob(ShaderType::VertexS) };

		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			builder.EmplaceBack(*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mVertices[i]),
								*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mNormals[i]),
								*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mTangents[i]),
								*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mBitangents[i]),
								*reinterpret_cast<DirectX::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
			);
		}

		Add(builder.Release());
	}
	else if (HasDiffuse)
	{
		BufferLayout layout{
		{ LayoutElement::ElementType::Position3 },
		{ LayoutElement::ElementType::Normal },
		{ LayoutElement::ElementType::TexCoords}
		};

		VertexBufferBuilder builder{ Name + "VertexBufferModel", std::move(layout), Shaders.GetBlob(ShaderType::VertexS) };

		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			builder.EmplaceBack(*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mVertices[i]),
								*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mNormals[i]),
								*reinterpret_cast<DirectX::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
			);
		}

		Add(builder.Release());
	}
	else
	{
		BufferLayout layout{
		{ LayoutElement::ElementType::Position3 },
		{ LayoutElement::ElementType::Normal }
		};

		VertexBufferBuilder builder{ Name + "VertexBufferModel", std::move(layout), Shaders.GetBlob(ShaderType::VertexS) };

		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			builder.EmplaceBack(*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mVertices[i]),
								*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mNormals[i])
			);
		}
	
		Add(builder.Release());
	}

	std::vector<unsigned short> indices;
	indices.reserve(mesh.mNumFaces * 3);
	for (unsigned int i = 0; i < mesh.mNumFaces; i++)
	{
		const auto& face = mesh.mFaces[i];
		assert(face.mNumIndices == 3);
		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
	}

	Add<IndexBuffer>(Name + "IndexBufferModel", indices);
}

Node::Node(Model& actor, const std::string& name)
	:NodeBase(actor, name)
{}

inline void Node::SetTransform(DirectX::XMMATRIX transform)
{
	const_cast<DirectX::XMMATRIX&>(Owner.Transform.GetMatrix()) = transform;
}

inline DirectX::XMMATRIX Node::GetTransform() const
{
	return Owner.GetTransform();
}

void Node::Update()
{
	for (auto* mesh : Meshes)
	{
		mesh->SetTransform(GetTransform());
	}

	for (auto& child : Children)
	{
		child->SetTransform(GetTransform() * child->GetRelativeTransform());
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

void Node::ShowTree()
{
	int trackedIndex = 0;
	NodeBase::ShowTree(trackedIndex, SelectedIndex, SelectedNode);

	if (SelectedNode)
		SelectedNode->GUITransform();
}

UniquePtr<Node> Node::Build(Model& actor, const std::string& filename)
{
	Assimp::Importer imp;
	std::filesystem::path solutionPath = std::filesystem::current_path().parent_path();

	const auto scene = imp.ReadFile(solutionPath.string() + "\\Content\\Model\\" + filename,
									aiProcess_Triangulate |
									aiProcess_JoinIdenticalVertices |
									aiProcess_ConvertToLeftHanded |
									aiProcess_GenNormals |
									aiProcess_CalcTangentSpace
	);

	auto* materials = scene->HasMaterials() ? scene->mMaterials : nullptr;

	auto& node = *scene->mRootNode;
	UniquePtr<Node> customNode = MakeUnique<Node>(actor, node.mName.C_Str());

	std::vector<Mesh*> meshes;

	for (size_t i = 0; i < node.mNumMeshes; i++)
	{
		const auto index = node.mMeshes[i];
		const auto& mesh = *scene->mMeshes[index];
		materials ? meshes.emplace_back(new Mesh(mesh, mesh.mName.C_Str(), materials, customNode->Owner.GetPath())) :
			meshes.emplace_back(new Mesh(mesh, mesh.mName.C_Str()));
	}

	customNode->Meshes = std::move(meshes);
	for (size_t i = 0; i < node.mNumChildren; i++)
		customNode->SetupChild(BuildImpl(*scene, *node.mChildren[i], materials, customNode->Owner));

	return std::move(customNode);
}

void Node::SetupChild(UniquePtr<NodeInternal> child)
{
	ASSERT(child);
	child->Parent = this;
	Children.emplace_back(std::move(child));
}

inline void Node::GUITransform()
{
	float roll = DirectX::XMConvertToRadians(Owner.Roll);
	float pitch = DirectX::XMConvertToRadians(Owner.Pitch);
	float yaw = DirectX::XMConvertToRadians(Owner.Yaw);

	float& x = Owner.X;
	float& y = Owner.Y;
	float& z = Owner.Z;

	ImGui::NextColumn();
	ImGui::Text("Orientation");
	ImGui::SliderAngle("Roll", &roll, -180.0f, 180.0f);
	ImGui::SliderAngle("Pitch", &pitch, -180.0f, 180.0f);
	ImGui::SliderAngle("Yaw", &yaw, -180.0f, 180.0f);

	ImGui::Text("Position");
	ImGui::SliderFloat("X", &x, -20.0f, 20.0f);
	ImGui::SliderFloat("Y", &y, -20.0f, 20.0f);
	ImGui::SliderFloat("Z", &z, -20.0f, 20.0f);

	Owner.X = x;
	Owner.Y = y;
	Owner.Z = z;

	roll = DirectX::XMConvertToDegrees(roll);
	pitch = DirectX::XMConvertToDegrees(pitch);
	yaw = DirectX::XMConvertToDegrees(yaw);

	Owner.Roll = roll;
	Owner.Pitch = pitch;
	Owner.Yaw = yaw;
}

inline UniquePtr<NodeInternal> Node::BuildImpl(const aiScene& scene, const aiNode& node, const aiMaterial* const* materials, Model& owner)
{
	const auto relativeTransform = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&node.mTransformation));
	std::vector<Mesh*> meshes;

	for (size_t i = 0; i < node.mNumMeshes; i++)
	{
		const auto index = node.mMeshes[i];
		const auto& mesh = *scene.mMeshes[index];
		materials ? meshes.emplace_back(new Mesh(mesh, mesh.mName.C_Str(), materials, owner.GetPath())) :
			meshes.emplace_back(new Mesh(mesh, mesh.mName.C_Str()));
	}

	UniquePtr<NodeInternal> customNode = MakeUnique<NodeInternal>(owner, node.mName.C_Str());
	customNode->SetRelativeTransform(relativeTransform);
	customNode->Meshes = std::move(meshes);

	for (size_t i = 0; i < node.mNumChildren; i++)
		customNode->SetupChild(BuildImpl(scene, *node.mChildren[i], materials, owner));

	return std::move(customNode);
}

NodeBase::NodeBase(Model& owner, const std::string& name)
	:Name(name), Owner(owner), Children{}, Meshes{}
{}

inline void NodeBase::ShowTree(int& trackedIndex, std::optional<int>& selectedIndex, NodeBase*& selectedNode) const
{
	const int currentNodeIndex = trackedIndex;
	trackedIndex++;

	const auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow
		| ((currentNodeIndex == selectedIndex.value_or(-1)) ? ImGuiTreeNodeFlags_Selected : 0)
		| ((Children.size() == 0) ? ImGuiTreeNodeFlags_Leaf : 0);

	const auto expanded = ImGui::TreeNodeEx((void*)(intptr_t)currentNodeIndex, node_flags, Name.c_str());

	if (ImGui::IsItemClicked())
	{
		selectedIndex = currentNodeIndex;
		selectedNode = const_cast<NodeBase*>(reinterpret_cast<const NodeBase*>(this));
	}

	if (expanded)
	{
		selectedIndex = ImGui::IsItemClicked() ? currentNodeIndex : selectedIndex;
		for (const auto& child : Children)
			child->ShowTree(trackedIndex, selectedIndex, selectedNode);

		ImGui::TreePop();
	}
}