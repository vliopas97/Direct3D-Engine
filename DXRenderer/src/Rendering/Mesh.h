#pragma once

#include "Rendering/Buffer.h"
#include "Rendering/Component.h"
#include "Rendering/CurrentGraphicsContext.h"
#include "Rendering/Shader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>

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

	Node();

	void SetTransform(DirectX::XMMATRIX transform);

	DirectX::XMMATRIX GetTransform() const;

	void SetRelativeTransform(DirectX::XMMATRIX transform);

	DirectX::XMMATRIX GetRelativeTransform() const;

	void SetupAttachment(Node* parent);

	void Update();

	void Draw();

	static UniquePtr<Node> Build(const std::string& filename);

private:
	void SetupChild(UniquePtr<Node> child);

	static UniquePtr<Node> BuildImpl(const aiScene& scene, const aiNode& node);

private:
	Node* Parent = nullptr;
	std::vector<UniquePtr<Node>> Children;
	std::vector<Mesh*> Meshes;

	DirectX::XMFLOAT4X4 Transform;
	DirectX::XMFLOAT4X4 RelativeTransform;
};