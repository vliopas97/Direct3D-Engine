#pragma once

#include "Rendering/Buffer.h"
#include "Rendering/Component.h"
#include "Rendering/CurrentGraphicsContext.h"
#include "Rendering/Shader.h"
#include "Rendering/Utilities.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>
#include <optional>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <imgui.h>

class Model;

class PrimitiveComponent : public Component, public GPUObject
{
public:
	PrimitiveComponent();

	void SetTransform(DirectX::XMMATRIX transform);
	DirectX::XMMATRIX GetTransform() const;

protected:
	DirectX::XMFLOAT4X4 Transform;

private:
	bool IsRootComponent = false;
};

class Mesh : public PrimitiveComponent
{
public:
	Mesh(const aiMesh& mesh, const std::string& meshName);
	Mesh(const aiMesh& mesh, const std::string& meshName, const aiMaterial* const* materials, const std::string& path);

	void Bind() const override;
	void Draw();


private:
	void Init(const aiMesh& mesh);
	void LoadMaterial(const aiMesh& mesh, const aiMaterial* const* materials, const std::string& path);
	std::pair<const char*, const char*> ResolveShaders() const;
	void ResolveVertexIndexBuffers(const aiMesh& mesh);

private:
	std::string Name;

	bool HasMaterial = false;
	bool HasDiffuse = false;
	bool HasAlphaDiffuse = false;
	bool HasNormals = false;
	bool HasSpecular = false;

	float Shininess = 2.0f;
};

class NodeBase
{
public:

	NodeBase(Model& owner, const std::string& name);
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
	Model& Owner;
};

class Node : public NodeBase
{
public:

	Node(Model& actor, const std::string& name = "Unknown");

	void SetTransform(DirectX::XMMATRIX transform) override;

	DirectX::XMMATRIX GetTransform() const override;
	//void SetupAttachment(Node* parent);

	void Update();
	void Draw();

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