#include "Mesh.h"
#include "Utilities.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

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

Node::Node(const std::string& name)
	:Children{}, Meshes{}, Name(name)
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
	DirectX::XMStoreFloat4x4(&RelativeTransform, transform);

	if (Parent)
		SetTransform(Parent->GetTransform() * GetRelativeTransform());
	else
		SetTransform(GetRelativeTransform());
}

DirectX::XMMATRIX Node::GetRelativeTransform() const
{
	return DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&RelativeTransform));
}

//inline void Node::SetupAttachment(Node* parent)
//{
//	if (parent && parent != Parent && parent != this)
//	{
//		auto* oldParent = Parent;
//		Parent = parent;
//
//		auto it = std::find_if(oldParent->Children.begin(), oldParent->Children.end(), [this](auto& uniquePtr)
//			{
//				return uniquePtr.get() == this;
//			});
//
//		ASSERT(it != oldParent->Children.end());
//		Parent->Children.emplace_back(std::unique_ptr<Node>(it->release()));
//		oldParent->Children.erase(it);
//	}
//}

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

void Node::GUITransform(const Node* root)
{
	if (!(this->Parent))
		return;

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

	DirectX::XMMATRIX newTransform = DirectX::XMMatrixRotationRollPitchYaw(-pitch, -yaw, roll) *
		DirectX::XMMatrixTranslation(-x, -y, z);
	SetRelativeTransform(newTransform);
}

void Node::ShowTree()
{
	int trackedIndex = 0;
	ShowTree(trackedIndex, SelectedIndex, SelectedNode);

	if (SelectedNode)
		SelectedNode->GUITransform(this);
}

void Node::ShowTree(int& trackedIndex, std::optional<int>& selectedIndex, Node*& selectedNode) const
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
		selectedNode = const_cast<Node*>(this);
	}

	if (expanded)
	{
		selectedIndex = ImGui::IsItemClicked() ? currentNodeIndex : selectedIndex;
		for(const auto& child : Children)
			child->ShowTree(trackedIndex, selectedIndex, selectedNode);

		ImGui::TreePop();
	}
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
	child->Parent = this;
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

	UniquePtr<Node> customNode = MakeUnique<Node>(node.mName.C_Str());
	customNode->SetRelativeTransform(relativeTransform);
	customNode->Meshes = std::move(meshes);

	for (size_t i = 0; i < node.mNumChildren; i++)
	{
		customNode->SetupChild(BuildImpl(scene, *node.mChildren[i]));
	}

	return std::move(customNode);
}
