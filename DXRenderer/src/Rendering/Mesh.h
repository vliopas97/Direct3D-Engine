#pragma once

#include "Rendering/Buffer.h"
#include "Rendering/Component.h"
#include "Rendering/CurrentGraphicsContext.h"
#include "Rendering/Shader.h"
#include "Rendering/Utilities.h"
#include "Rendering/RenderQueue.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>


class Model;

class PrimitiveComponent : public Component, public GPUObject
{
public:
	PrimitiveComponent();

	void SetTransform(DirectX::XMMATRIX transform);
	DirectX::XMMATRIX GetTransform() const;
	virtual void Tick(float delta) override;

protected:
	DirectX::XMFLOAT4X4 Transform;
	DirectX::XMFLOAT4X4 ModelView;

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
	void Init(const aiMesh& mesh, Step& first);
	void LoadMaterial(const aiMesh& mesh, const aiMaterial* const* materials, const std::string& path, Step& first);
	std::pair<const char*, const char*> ResolveShaders() const;

private:
	std::string Name;

	bool HasMaterial = false;
	bool HasDiffuse = false;
	bool HasAlphaDiffuse = false;
	bool HasNormals = false;
	bool HasSpecular = false;

	float Shininess = 2.0f;
};