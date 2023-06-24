#pragma once

#include "Core\Core.h"
#include "CurrentGraphicsContext.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <wrl.h>

#include "Core\Exception.h"

struct VertexElement
{
	VertexElement() = default;
	VertexElement(float x, float y, float z);
	DirectX::XMFLOAT3 Position;
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

	enum class ElementType
	{
		Position2, Position3,
		Color3, Color4,
		Normal,
		TexCoords,
	};

public:
	LayoutElement(const std::string& name, DataType type);
	LayoutElement(ElementType type);

	uint32_t GetSize() const { return Size; }
	uint32_t GetOffset() const { return Offset; }
	ElementType GetType() const { return Type; }

private:
	static const char* ResolveNameFromType(ElementType type);
	static DXGI_FORMAT DataTypeToDXGI(DataType type);
	static DXGI_FORMAT DataTypeToDXGI(ElementType type);
	static uint32_t CalcSize(DataType type);
	static uint32_t CalcSize(ElementType type);

private:
	ElementType Type;
	std::string Name;
	DXGI_FORMAT Format;
	uint32_t Size;
	uint32_t Offset;

	friend class VertexBuffer;
	friend struct BufferLayout;
};

struct BufferLayout
{
	BufferLayout() = default;
	BufferLayout(std::initializer_list<LayoutElement> elements);

	BufferLayout& AddElement(LayoutElement element);

	uint32_t GetStride() const;
	size_t GetElementsSize() const;

	template<LayoutElement::ElementType Type>
	const LayoutElement& ResolveType() const
	{
		for (auto& element : Elements)
		{
			if (element.GetType() == Type)
				return element;
		}
		ASSERT(false);
		return Elements.front();
	}

	const LayoutElement& operator[](size_t i) const;
private:

	std::vector<LayoutElement> Elements;
	uint32_t Stride = 0;

	friend class VertexBuffer;
};

struct Vertex
{
public:
	template<typename T>
	void SetAttributeIndex(size_t i, T&& attr)
	{
		const auto& element = Layout[i];
		auto attributePtr = Ptr + element.GetOffset();

		switch (element.GetType())
		{
				case LayoutElement::ElementType::Position2: SetAttribute<DirectX::XMFLOAT2>(attributePtr, std::forward<T>(attr)); break;
				case LayoutElement::ElementType::Position3:SetAttribute<DirectX::XMFLOAT3>(attributePtr, std::forward<T>(attr)); break;
				case LayoutElement::ElementType::Color3: SetAttribute<DirectX::XMFLOAT3>(attributePtr, std::forward<T>(attr)); break;
				case LayoutElement::ElementType::Color4: SetAttribute<DirectX::XMFLOAT4>(attributePtr, std::forward<T>(attr)); break;
				case LayoutElement::ElementType::Normal: SetAttribute<DirectX::XMFLOAT3>(attributePtr, std::forward<T>(attr)); break;
				case LayoutElement::ElementType::TexCoords: SetAttribute<DirectX::XMFLOAT2>(attributePtr, std::forward<T>(attr)); break;
		}
	}

private:
	Vertex(char* ptr, const BufferLayout& layout);

	template<typename First, typename ...Rest>
	void SetAttributeIndex(size_t i, First&& first, Rest&&... rest)
	{
		SetAttributeIndex(i, std::forward<First>(first));
		SetAttributeIndex(i + 1, std::forward<Rest>(rest)...);
	}

	template<typename Dest, typename Src>
	void SetAttribute(char* attributePtr, Src&& attr)
	{
		if constexpr (std::is_assignable<Dest, Src>::value)
			*reinterpret_cast<Dest*>(attributePtr) = attr;
		else
			ASSERT(false);
	}
private:
	char* Ptr;
	const BufferLayout& Layout;

	friend class VertexBufferBuilder;
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
	VertexBuffer(const std::vector<Vertex>& vertices, BufferLayout&& layout, const Microsoft::WRL::ComPtr<ID3DBlob>& blob)
		:Layout(std::move(layout)), Blob(blob), Topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
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

	VertexBuffer(BufferLayout&& layout, const Microsoft::WRL::ComPtr<ID3DBlob>& blob);

	void Bind() const override;
	void Unbind() const override;

	BufferType GetType() const;

public:
	D3D11_PRIMITIVE_TOPOLOGY Topology;

private:
	void BindLayout() const;

private:
	BufferLayout Layout;
	const Microsoft::WRL::ComPtr<ID3DBlob>& Blob;

	static const BufferType Type = BufferType::VertexB;

	friend class VertexBufferBuilder;
};

class VertexBufferBuilder
{
public:
	VertexBufferBuilder(BufferLayout&& layout, const Microsoft::WRL::ComPtr<ID3DBlob>& blob);

	template<typename ...Attributes>
	void EmplaceBack(Attributes&&... attributes)
	{
		Vertices.resize(Vertices.size() + Object.Layout.GetStride());
		Back().SetAttributeIndex(0, std::forward<Attributes>(attributes)...);
	}

	UniquePtr<VertexBuffer> Release();

private:

	Vertex Back();

	Vertex Front();
private:
	std::vector<char> Vertices{};
	VertexBuffer Object;
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
		DXGIInfoManager InfoManager;

		D3D11_BUFFER_DESC constantBufferDesc{};
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		constantBufferDesc.MiscFlags = 0;
		constantBufferDesc.ByteWidth = (sizeof(T) % 16 == 0) ? sizeof(T) : sizeof(T) + (16 - sizeof(T) % 16);
		constantBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA subResourceData{};
		subResourceData.pSysMem = &Resource;

		GRAPHICS_ASSERT(CurrentGraphicsContext::Device()->CreateBuffer(&constantBufferDesc, &subResourceData, &BufferID));
	}

	BufferType GetType() const { return Type; }
	T& GetResource() { return Resource; }

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
	//Uniform(const UniquePtr<ConstantBuffer<T>>& constantBuffer, const T& resource)
	//	:ConstantBufferRef(const_cast<UniquePtr<ConstantBuffer<T>>&>(constantBuffer).release()), Resource(resource)
	//{
	//	BufferID = ConstantBufferRef->BufferID;
	//}

	Uniform(UniquePtr < ConstantBuffer<T>> constantBuffer, const T& resource)
		:ConstantBufferRef(std::move(constantBuffer)), Resource(resource)
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