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

class Node
{
public:

	Node(const std::string& name = "Unknown");

	void SetTransform(DirectX::XMMATRIX transform);

	DirectX::XMMATRIX GetTransform() const;
	void SetRelativeTransform(DirectX::XMMATRIX transform);
	DirectX::XMMATRIX GetRelativeTransform() const;

	//void SetupAttachment(Node* parent);

	void Update();
	void Draw();

	void ShowTree();

	static UniquePtr<Node> Build(const std::string& filename);

private:
	void SetupChild(UniquePtr<Node> child);
	void ShowTree(int& trackedIndex, std::optional<int>& selectedIndex, Node*& selectedNode) const;
	void GUITransform(const Node* root);

	static UniquePtr<Node> BuildImpl(const aiScene& scene, const aiNode& node);

private:
	const Node* Parent = nullptr;
	std::vector<UniquePtr<Node>> Children;
	std::vector<Mesh*> Meshes;
	std::string Name;

	DirectX::XMFLOAT4X4 Transform;
	DirectX::XMFLOAT4X4 RelativeTransform;

	std::optional<int> SelectedIndex;
	mutable Node* SelectedNode = nullptr;
};