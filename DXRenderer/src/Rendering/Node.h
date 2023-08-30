#pragma once

#include "Core/Core.h"
#include "Rendering/Mesh.h"
#include <optional>


#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <imgui.h>

class NodeBase
{
public:

	NodeBase(Model& owner, const std::string& name);
	virtual ~NodeBase() = default;

	virtual void SetTransform(DirectX::XMMATRIX transform) = 0;

	virtual DirectX::XMMATRIX GetTransform() const = 0;

	virtual void GUITransform() = 0;
	void Tick(float delta);
	void LinkTechniques();

protected:
	void ShowTree(int& trackedIndex, std::optional<int>& selectedIndex, NodeBase*& selectedNode) const;

protected:
	std::vector<Mesh*> Meshes;
	std::vector<UniquePtr<class NodeInternal>> Children;

	std::string Name;
	Model& Owner;
};

class Node : public NodeBase
{
public:

	Node(Model& actor, const std::string& name = "Unknown");

	void SetTransform(DirectX::XMMATRIX transform) override;

	DirectX::XMMATRIX GetTransform() const override;
	//void SetupAttachment(Node* parent);

	void Submit(size_t channelsIn);

	void ShowTree();

	static UniquePtr<Node> Build(Model& actor, const std::string& filename);

private:
	void SetupChild(UniquePtr<class NodeInternal> child);
	void GUITransform() override;

	static UniquePtr<class NodeInternal> BuildImpl(const aiScene& scene, const aiNode& node, const aiMaterial* const* materials,
												   Model& owner);

private:
	std::optional<int> SelectedIndex;
	mutable NodeBase* SelectedNode = nullptr;
};
