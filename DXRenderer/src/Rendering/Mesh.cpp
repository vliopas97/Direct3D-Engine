#include "Mesh.h"
#include "Utilities.h"

#include "Actors/Actor.h"



class NodeInternal : public NodeBase
{
public:
	NodeInternal(const std::string& name = "Unknown")
		:NodeBase(name)
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

		if (Parent)
			SetTransform(Parent->GetTransform() * GetRelativeTransform());
		else
			SetTransform(GetRelativeTransform());
	}

	DirectX::XMMATRIX GetRelativeTransform() const
	{
		return DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&RelativeTransform));
	}

	void Update()
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

	void NodeBase::GUITransform() override
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

		DirectX::XMMATRIX newTransform = DirectX::XMMatrixRotationRollPitchYaw(-pitch, -yaw, roll) *
			DirectX::XMMatrixTranslation(-x, -y, z);
		SetRelativeTransform(newTransform);
	}

private:
	const NodeBase* Parent = nullptr;

	DirectX::XMFLOAT4X4 Transform;
	DirectX::XMFLOAT4X4 RelativeTransform;

	friend class Node;
};

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

Node::Node(const Actor& actor, const std::string& name)
	:Owner(const_cast<Actor&>(actor)), NodeBase(name)
{
}

inline void Node::SetTransform(DirectX::XMMATRIX transform)
{
	const_cast<DirectX::XMMATRIX&>(Owner.Transform.GetMatrix()) = transform;
}

inline DirectX::XMMATRIX Node::GetTransform() const
{
	return Owner.GetTransform();
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
	//glm::mat tran = *reinterpret_cast<const glm::mat4x4*>(&RelativeTransform);

	//glm::vec3 translate(tran[3]);
	//translate.x *= -1;
	//translate.y *= -1;

	//tran = glm::transpose(tran);
	//glm::quat quaternion = glm::quat_cast(tran);
	//glm::vec3 angles = glm::eulerAngles(quaternion);
	//angles.z *= -1;

	//Owner.Roll = angles.z;
	//Owner.Pitch = angles.x;
	//Owner.Yaw = angles.y;

	//Owner.X = translate.x;
	//Owner.Y = translate.y;
	//Owner.Z = translate.z;

	auto& transform = Owner.Transform.GetMatrix();
	for (auto* mesh : Meshes)
	{
		mesh->SetTransform(transform);
	}

	for (auto& child : Children)
	{
		child->SetTransform(transform * child->GetRelativeTransform());
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

//void Node::GUITransform()
//{
//	glm::mat transform = *reinterpret_cast<const glm::mat4x4*>(&RelativeTransform);
//
//	glm::vec3 translate(transform[3]);
//	translate.x *= -1;
//	translate.y *= -1;
//
//	transform = glm::transpose(transform);
//	glm::quat quaternion = glm::quat_cast(transform);
//	glm::vec3 angles = glm::eulerAngles(quaternion);
//	angles.z *= -1;
//
//	// Convert the quaternion to Euler angles
//	float& roll = angles.z;
//	float& pitch = angles.x;
//	float& yaw = angles.y;
//
//	float& x = translate.x;
//	float& y = translate.y;
//	float& z = translate.z;
//
//	ImGui::NextColumn();
//	ImGui::Text("Orientation (Relative)");
//	ImGui::SliderAngle("Roll", &roll, -180.0f, 180.0f);
//	ImGui::SliderAngle("Pitch", &pitch, -180.0f, 180.0f);
//	ImGui::SliderAngle("Yaw", &yaw, -180.0f, 180.0f);
//
//	ImGui::Text("Position (Relative)");
//	ImGui::SliderFloat("X", &x, -20.0f, 20.0f);
//	ImGui::SliderFloat("Y", &y, -20.0f, 20.0f);
//	ImGui::SliderFloat("Z", &z, -20.0f, 20.0f);
//
//	DirectX::XMMATRIX newTransform = DirectX::XMMatrixRotationRollPitchYaw(-pitch, -yaw, roll) *
//		DirectX::XMMatrixTranslation(-x, -y, z);
//	SetRelativeTransform(newTransform);
//}

void Node::ShowTree()
{
	int trackedIndex = 0;
	NodeBase::ShowTree(trackedIndex, SelectedIndex, SelectedNode);

	if (SelectedNode)
		SelectedNode->GUITransform();
}

UniquePtr<Node> Node::Build(const Actor& actor, const std::string& filename)
{
	Assimp::Importer imp;
	std::filesystem::path solutionPath = std::filesystem::current_path().parent_path();

	const auto scene = imp.ReadFile(solutionPath.string() + "\\Content\\Model\\" + filename,
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices
	);

	auto& node = *scene->mRootNode;
	UniquePtr<Node> customNode = MakeUnique<Node>(actor, node.mName.C_Str());

	std::vector<Mesh*> meshes;

	for (size_t i = 0; i < node.mNumMeshes; i++)
	{
		const auto index = node.mMeshes[i];
		meshes.emplace_back(new Mesh(*scene->mMeshes[index]));
	}

	customNode->Meshes = std::move(meshes);
	for (size_t i = 0; i < node.mNumChildren; i++)
		customNode->SetupChild(BuildImpl(*scene, *node.mChildren[i]));

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
	auto& ownerTransform = Owner.Transform.GetMatrix();
	glm::mat transform = *reinterpret_cast<const glm::mat4x4*>(&ownerTransform);

	glm::vec3 translate(transform[3]);
	translate.x *= -1;
	translate.y *= -1;

	transform = glm::transpose(transform);
	glm::quat quaternion = glm::quat_cast(transform);
	glm::vec3 angles = glm::eulerAngles(quaternion);
	angles.z *= -1;

	float& roll = angles.z;
	float& pitch = angles.x;
	float& yaw = angles.y;

	float& x = translate.x;
	float& y = translate.y;
	float& z = translate.z;

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

	angles = glm::degrees(angles);
	Owner.Roll = roll;
	Owner.Pitch = pitch;
	Owner.Yaw = yaw;
}

inline UniquePtr<NodeInternal> Node::BuildImpl(const aiScene& scene, const aiNode& node)
{
	const auto relativeTransform = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&node.mTransformation));
	std::vector<Mesh*> meshes;

	for (size_t i = 0; i < node.mNumMeshes; i++)
	{
		const auto index = node.mMeshes[i];
		meshes.emplace_back(new Mesh(*scene.mMeshes[index]));
	}

	UniquePtr<NodeInternal> customNode = MakeUnique<NodeInternal>(node.mName.C_Str());
	customNode->SetRelativeTransform(relativeTransform);
	customNode->Meshes = std::move(meshes);

	for (size_t i = 0; i < node.mNumChildren; i++)
		customNode->SetupChild(BuildImpl(scene, *node.mChildren[i]));

	return std::move(customNode);
}

NodeBase::NodeBase(const std::string& name)
	:Name(name), Children{}, Meshes{}
{
}

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
