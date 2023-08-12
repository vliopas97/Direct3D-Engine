#include "Mesh.h"
#include "Utilities.h"

#include "Actors/Model.h"
#include "Rendering/Material.h"
#include "Rendering/State.h"

inline PrimitiveComponent::PrimitiveComponent()
{
	DirectX::XMStoreFloat4x4(&Transform, DirectX::XMMatrixIdentity());
}

void PrimitiveComponent::SetTransform(DirectX::XMMATRIX transform)
{
	DirectX::XMStoreFloat4x4(&Transform, transform);
}

inline DirectX::XMMATRIX PrimitiveComponent::GetTransform() const
{
	return DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&Transform));
}

inline void PrimitiveComponent::Tick(float delta)
{
	DirectX::XMStoreFloat4x4(&ModelView,
							 GetTransform() * CurrentGraphicsContext::GraphicsInfo->GetCamera().GetView());
}

Mesh::Mesh(const aiMesh& mesh, const std::string& meshName)
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

void Mesh::Draw()
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
	Add<UniformPS<XMMATRIX>>(Name + "View", view, 2);

	const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetCamera().GetProjection();
	Add<UniformVS<XMMATRIX>>(Name + "Proj", projection, 1);

	auto& modelView = *reinterpret_cast<const XMMATRIX*>(&ModelView);
	Add<UniformVS<XMMATRIX>>(Name + "Transform", modelView);

	if (!HasSpecular)
	{
		auto material = MakeUnique<Material>(1);
		material->Properties.Shininess = Shininess;
		Add(std::move(material));
	}

	Add<RasterizerState>(HasAlphaDiffuse);
	Add<StencilState<>>(Name + "DepthStencilState");
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
		auto ptr = builder.Release();
		Add<InputLayout>(Name, ptr->GetLayout(), Shaders.GetBlob(ShaderType::VertexS));
		Add(std::move(ptr));
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

		auto ptr = builder.Release();
		Add<InputLayout>(Name, ptr->GetLayout(), Shaders.GetBlob(ShaderType::VertexS));
		Add(std::move(ptr));
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
	
		auto ptr = builder.Release();
		Add<InputLayout>(Name, ptr->GetLayout(), Shaders.GetBlob(ShaderType::VertexS));
		Add(std::move(ptr));
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