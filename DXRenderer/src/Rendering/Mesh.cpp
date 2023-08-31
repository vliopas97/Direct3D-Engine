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

void PrimitiveComponent::Tick(float delta)
{
	DirectX::XMStoreFloat4x4(&ModelView,
							 GetTransform() * CurrentGraphicsContext::GraphicsInfo->GetView());
}

Mesh::Mesh(const aiMesh& mesh, const std::string& meshName)
	:Name(meshName)
{
	using namespace DirectX;
	Technique standard(Channels::Main);
	Step first("phong");

	auto [vertexName, pixelName] = ResolveShaders();
	VertexShader vertexShader(vertexName);
	first.Add<VertexShader>(vertexShader);
	first.Add<PixelShader>(pixelName);
	
	BufferLayout layout{
	{ LayoutElement::ElementType::Position3 },
	{ LayoutElement::ElementType::Normal }
	};

	VertexBufferBuilder builder{ Name + "VertexBufferModel", std::move(layout), vertexShader.GetBlob() };

	for (unsigned int i = 0; i < mesh.mNumVertices; i++)
	{
		builder.EmplaceBack(*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mVertices[i]),
							*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mNormals[i])
		);
	}

	auto ptr = builder.Release();
	first.Add<InputLayout>(Name, ptr->GetLayout(), vertexShader.GetBlob());
	Add(std::move(ptr));

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

	const DirectX::XMMATRIX& view = CurrentGraphicsContext::GraphicsInfo->GetView();
	first.Add<UniformPS<XMMATRIX>>(Name + "View", view, 2);

	const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetProjection();
	first.Add<UniformVS<XMMATRIX>>(Name + "Proj", projection, 1);

	auto& modelView = *reinterpret_cast<const XMMATRIX*>(&ModelView);
	first.Add<UniformVS<XMMATRIX>>(Name + "Transform" + UIDTag(), modelView);

	if (!HasSpecular)
	{
		auto material = Material(1);
		material.Properties.Shininess = Shininess;
		first.Add<Material>(std::move(material));
	}

	first.Add<RasterizerState>(HasAlphaDiffuse);

	standard.PushBack(std::move(first));
	Add(std::move(standard));
}

Mesh::Mesh(const aiMesh& mesh, const std::string& meshName, const aiMaterial* const* materials, const std::string& path)
	:Name(meshName)
{
	ASSERT(materials);
	using namespace DirectX;

	DirectX::XMFLOAT4 diffuseColor = { 0.45f,0.45f,0.85f,1.0f };

	HasMaterial = mesh.mMaterialIndex >= 0;
	if (!HasMaterial)
		return;

	auto& material = materials[mesh.mMaterialIndex];
	aiString filename;
	std::array<UniquePtr<Texture>, 3> textures{ nullptr };
	if (material->GetTexture(aiTextureType_DIFFUSE, 0, &filename) == aiReturn_SUCCESS)
	{
		HasDiffuse = true;
		auto t = MakeUnique<Texture>(path + filename.C_Str());
		HasAlphaDiffuse = t->HasAlpha();
		//first.Add<Texture>(std::move(t));
		textures[0] = std::move(t);
	}
	else
		material->Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor3D&>(diffuseColor));

	if (material->GetTexture(aiTextureType_NORMALS, 0, &filename) == aiReturn_SUCCESS)
	{
		HasNormals = true;
		//first.Add<Texture>(path + filename.C_Str(), 1);
		textures[1] = MakeUnique<Texture>(path + filename.C_Str(), 1);
	}

	if (material->GetTexture(aiTextureType_SPECULAR, 0, &filename) == aiReturn_SUCCESS)
	{
		HasSpecular = true;
		//first.Add<Texture>(path + filename.C_Str(), 2);
		textures[2] = MakeUnique<Texture>(path + filename.C_Str(), 2);
	}
	else
		material->Get(AI_MATKEY_SHININESS, Shininess);

	auto [vertexName, pixelName] = ResolveShaders();
	VertexShader vertexShader(vertexName);
	SharedPtr<VertexBuffer> vertexBuffer;

	if (HasDiffuse && HasNormals)
	{
		BufferLayout layout{
		{ LayoutElement::ElementType::Position3 },
		{ LayoutElement::ElementType::Normal },
		{ LayoutElement::ElementType::Tangent},
		{ LayoutElement::ElementType::Bitangent},
		{ LayoutElement::ElementType::TexCoords}
		};

		VertexBufferBuilder builder{ Name + "VertexBufferModel", std::move(layout), vertexShader.GetBlob() };

		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			builder.EmplaceBack(*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mVertices[i]),
								*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mNormals[i]),
								*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mTangents[i]),
								*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mBitangents[i]),
								*reinterpret_cast<DirectX::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
			);
		}
		vertexBuffer = builder.Release();
		Add(vertexBuffer);
	}
	else if (HasDiffuse)
	{
		BufferLayout layout{
		{ LayoutElement::ElementType::Position3 },
		{ LayoutElement::ElementType::Normal },
		{ LayoutElement::ElementType::TexCoords}
		};

		VertexBufferBuilder builder{ Name + "VertexBufferModel", std::move(layout), vertexShader.GetBlob() };

		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			builder.EmplaceBack(*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mVertices[i]),
								*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mNormals[i]),
								*reinterpret_cast<DirectX::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
			);
		}

		vertexBuffer = builder.Release();
		Add(vertexBuffer);
	}
	else
	{
		BufferLayout layout{
		{ LayoutElement::ElementType::Position3 },
		{ LayoutElement::ElementType::Normal }
		};

		VertexBufferBuilder builder{ Name + "VertexBufferModel", std::move(layout), vertexShader.GetBlob() };

		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			builder.EmplaceBack(*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mVertices[i]),
								*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mNormals[i])
			);
		}

		vertexBuffer = builder.Release();
		Add(vertexBuffer);
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

	Technique standard(Channels::Main);
	{
		Step first("phong");

		first.Add<VertexShader>(vertexShader);
		first.Add<PixelShader>(pixelName);
		first.Add<InputLayout>(Name, vertexBuffer->GetLayout(), vertexShader.GetBlob());

		const DirectX::XMMATRIX& view = CurrentGraphicsContext::GraphicsInfo->GetView();
		first.Add<UniformPS<XMMATRIX>>(Name + "View", view, 2);

		auto& model = *reinterpret_cast<const XMMATRIX*>(&Transform);
		first.Add<UniformVS<XMMATRIX>>(Name + "Model" + UIDTag(), model);

		auto& modelView = *reinterpret_cast<const XMMATRIX*>(&ModelView);
		first.Add<UniformVS<XMMATRIX>>(Name + "Transform" + UIDTag(), modelView, 1);

		const DirectX::XMMATRIX& projection = CurrentGraphicsContext::GraphicsInfo->GetProjection();
		first.Add<UniformVS<XMMATRIX>>(Name + "Proj", projection, 2);

		if (!HasSpecular)
		{
			auto material = Material(1);
			material.Properties.Shininess = Shininess;
			first.Add<Material>(std::move(material));
		}

		first.Add<RasterizerState>(HasAlphaDiffuse);

		for (auto& t : textures)
		{
			if (t)
				first.Add<Texture>(std::move(*t.release()));
		}

		standard.PushBack(std::move(first));
	}


	Technique shadowMap(Channels::Shadow);
	{
		Step draw("shadowMap");
		VertexShader vs("ShadowMapUpdate");
		draw.Add<InputLayout>(Name, vertexBuffer->GetLayout(), vs.GetBlob());

		auto& transform = *reinterpret_cast<const XMMATRIX*>(&Transform);
		draw.Add<UniformVS<XMMATRIX>>(Name + "Model" + UIDTag(), transform);
		shadowMap.PushBack(std::move(draw));
	}

	Add(std::move(standard));
	Add(std::move(shadowMap));
}

inline void Mesh::Bind() const
{
	Shaders.Bind();
	Buffers.Bind();
	Components.Bind();
}

void Mesh::Submit(size_t channelsIn)
{
	for (auto& t : Techniques)
		t->Submit(*this, channelsIn);
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