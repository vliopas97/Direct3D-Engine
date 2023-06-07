#pragma once

#include "Core\Core.h"
#include "CurrentGraphicsContext.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <wrl.h>

struct VertexElement
{
	struct
	{
		float x;
		float y;
		float z;
	} Position;

	struct
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
	} Color;
};

struct FaceColors
{
	struct
	{
		float r;
		float g;
		float b;
		float a;
	} face_colors[6];
};

struct LayoutElement
{
	enum class DataType
	{
		UChar, UChar2, UChar4,
		UCharNorm, UChar2Norm, UChar4Norm,
		Float, Float2, Float3, Float4,
		Int, Int2, Int3, Int4
	};

public:
	LayoutElement(const std::string& name, DataType type);

private:
	static DXGI_FORMAT DataTypeToDXGI(DataType type);
	static uint32_t CalcSize(DataType type);

public:
	std::string Name;
	DataType Type;
	uint32_t Offset;
	uint32_t Size;

	friend class VertexBuffer;
};

struct BufferLayout
{
	BufferLayout() = default;
	BufferLayout(std::initializer_list<LayoutElement> elements);

	BufferLayout& AddElement(LayoutElement element);

	uint32_t GetStride() const;
	size_t GetElementsSize() const;
private:

	std::vector<LayoutElement> Elements;
	uint32_t Stride = 0;

	friend class VertexBuffer;
};

enum BufferType
{
	VertexB = 0,
	IndexB,
	ConstantB
};

class Buffer
{
public:
	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;
	virtual ~Buffer() = default;

protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> BufferID;
};

class VertexBuffer : public Buffer
{
public:
	template<typename Vertex>
	VertexBuffer(const std::vector<Vertex>& vertices, const Microsoft::WRL::ComPtr<ID3DBlob>& blob)
		:Blob(blob), Topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.ByteWidth = sizeof(Vertex) * vertices.size();
		vertexBufferDesc.StructureByteStride = sizeof(Vertex);

		D3D11_SUBRESOURCE_DATA subResourceData;
		subResourceData.pSysMem = vertices.data();

		CurrentGraphicsContext::Device()->CreateBuffer(&vertexBufferDesc, &subResourceData, &BufferID);
	}

	void Bind() const override;
	void Unbind() const override;

	void SetLayout(const BufferLayout& layout);
	VertexBuffer& AddLayoutElement(LayoutElement element);

	BufferType GetType() const;

public:
	D3D11_PRIMITIVE_TOPOLOGY Topology;

private:
	void BindLayout() const;

private:
	BufferLayout Layout;
	const Microsoft::WRL::ComPtr<ID3DBlob>& Blob;

	static const BufferType Type = BufferType::VertexB;
};

class IndexBuffer : public Buffer
{
public:
	IndexBuffer(const std::vector<unsigned short>& indices);

	void Bind() const override;
	void Unbind() const override;

	BufferType GetType() const;
	UINT GetCount() const;
private:
	static const BufferType Type = BufferType::IndexB;
	UINT Count;
};

template<typename T>
class ConstantBuffer : public Buffer
{
public:
	virtual ~ConstantBuffer() = default;
	ConstantBuffer(const T& resource, uint32_t slot = 0)
		:Resource(resource), Slot(slot)
	{
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.ByteWidth = sizeof(Resource);
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA subResourceData;
		subResourceData.pSysMem = &Resource;

		CurrentGraphicsContext::Device()->CreateBuffer(&indexBufferDesc, &subResourceData, &BufferID);
	}

	BufferType GetType() const
	{
		return Type;
	}

protected:
	uint32_t Slot;
private:
	T Resource;

	static const BufferType Type = BufferType::ConstantB;

	template<typename T>
	friend class Uniform;
};

template<typename T>
class VSConstantBuffer : public ConstantBuffer<T>
{
public:
	using ConstantBuffer<T>::ConstantBuffer;
	using ConstantBuffer<T>::BufferID;
	using ConstantBuffer<T>::Slot;

	void Bind() const override
	{
		CurrentGraphicsContext::Context()->VSSetConstantBuffers(Slot, 1, BufferID.GetAddressOf());
	}
	void Unbind() const override
	{
		CurrentGraphicsContext::Context()->VSSetConstantBuffers(Slot, 1, nullptr);
	}
};

template<typename T>
class PSConstantBuffer : public ConstantBuffer<T>
{
public:
	using ConstantBuffer<T>::ConstantBuffer;
	using ConstantBuffer<T>::BufferID;
	using ConstantBuffer<T>::Slot;

	void Bind() const override
	{
		CurrentGraphicsContext::Context()->PSSetConstantBuffers(Slot, 1, BufferID.GetAddressOf());
	}

	void Unbind() const override
	{
		CurrentGraphicsContext::Context()->PSSetConstantBuffers(Slot, 1, nullptr);
	}
};

template<typename T>
class Uniform : public Buffer
{
public:
	Uniform(const UniquePtr<ConstantBuffer<T>>& constantBuffer, const T& resource)
		:ConstantBufferRef(const_cast<UniquePtr<ConstantBuffer<T>>&>(constantBuffer).release()), Resource(resource)
	{
		BufferID = ConstantBufferRef->BufferID;
	}

	void Bind() const override
	{
		Update();
		ConstantBufferRef->Bind();
	}

	void Unbind() const override
	{
		ConstantBufferRef->Unbind();
	}

private:
	void Update() const
	{
		D3D11_MAPPED_SUBRESOURCE subResource;
		CurrentGraphicsContext::Context()->Map(BufferID.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
		memcpy(subResource.pData, &Resource, sizeof(Resource));
		CurrentGraphicsContext::Context()->Unmap(BufferID.Get(), 0);
	}

private:
	UniquePtr<ConstantBuffer<T>> ConstantBufferRef;
	const T& Resource;
};

class DepthBuffer
{
public:
	DepthBuffer(Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& dSV);

	void Bind() const;
	void Unbind() const;

private:
	void CreateDepthStencilState();
	void CreateDepthStencilTexture();
	void CreateDepthStencilView(Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& dSV) const;
private:
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencil;
};

class BufferGroup : public Buffer
{
public:
	void Add(UniquePtr<Buffer> buffer);
	const IndexBuffer* GetIndexBuffer() const;

	virtual void Bind() const override;
	virtual void Unbind() const override;

	inline size_t Size() const { return Buffers.size(); }
private:
	std::vector<UniquePtr<Buffer>> Buffers;
};