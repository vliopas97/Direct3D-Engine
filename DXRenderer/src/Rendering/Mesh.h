#pragma once

#include "Rendering/Buffer.h"
#include "Rendering/Component.h"
#include "Rendering/CurrentGraphicsContext.h"
#include "Rendering/Shader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>
#include <optional>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <imgui.h>

class PrimitiveComponent : public Component
{
public:
	PrimitiveComponent();

	void SetTransform(DirectX::XMMATRIX transform);

	DirectX::XMMATRIX GetTransform() const;

protected:
	void Add(UniquePtr<Buffer> buffer);

	void Add(UniquePtr<Shader> shader);

protected:
	DirectX::XMFLOAT4X4 Transform;

	BufferGroup Buffers;
	ShaderGroup Shaders;

private:
	bool IsRootComponent = false;
};

class Mesh : public PrimitiveComponent
{
public:
	Mesh(const aiMesh& mesh);

	void Bind() const override;

	void Draw();
};

class NodeBase
{
public:

	NodeBase(const std::string& name);
	virtual ~NodeBase() = default;

	virtual void SetTransform(DirectX::XMMATRIX transform) = 0;

	virtual DirectX::XMMATRIX GetTransform() const = 0;

	virtual void GUITransform() = 0;

protected:
	void ShowTree(int& trackedIndex, std::optional<int>& selectedIndex, NodeBase*& selectedNode) const;

protected:
	std::vector<Mesh*> Meshes;
	std::vector<UniquePtr<class NodeInternal>> Children;

	std::string Name;
};

class Actor;

class Node : public NodeBase
{
public:

	Node(const Actor& actor, const std::string& name = "Unknown");

	void SetTransform(DirectX::XMMATRIX transform) override;

	DirectX::XMMATRIX GetTransform() const override;
	//void SetupAttachment(Node* parent);

	void Update();
	void Draw();

	void ShowTree();

	static UniquePtr<Node> Build(const Actor& actor, const std::string& filename);

private:
	void SetupChild(UniquePtr<class NodeInternal> child);
	void GUITransform() override;

	static UniquePtr<class NodeInternal> BuildImpl(const aiScene& scene, const aiNode& node);

private:
	std::optional<int> SelectedIndex;
	mutable NodeBase* SelectedNode = nullptr;
	Actor& Owner;
};