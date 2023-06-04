#pragma once

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
	VertexBuffer(const VertexElement* vertices, int size);

	void Bind() const override;
	void Unbind() const override;

	void SetLayout(const BufferLayout& layout);
	VertexBuffer& AddLayoutElement(LayoutElement element);

	void CreateLayout(const Microsoft::WRL::ComPtr<ID3DBlob>& blob);
private:
	BufferLayout Layout;
};

class IndexBuffer : public Buffer
{
public:
	IndexBuffer(const unsigned short* indices, int size);

	void Bind() const override;
	void Unbind() const override;
};


template<typename T>
class ConstantBuffer : public Buffer
{
public:
	virtual ~ConstantBuffer() = default;
	ConstantBuffer(const T& matrix)
		:Matrix(matrix)
	{
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.ByteWidth = sizeof(Matrix);
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA subResourceData;
		subResourceData.pSysMem = &Matrix;

		CurrentGraphicsContext::Device()->CreateBuffer(&indexBufferDesc, &subResourceData, &BufferID);
	}
private:
	T Matrix;
};

template<typename T>
class VSConstantBuffer : public ConstantBuffer<T>
{
public:
	using ConstantBuffer<T>::ConstantBuffer;
	using ConstantBuffer<T>::BufferID;

	void Bind() const override
	{
		CurrentGraphicsContext::Context()->VSSetConstantBuffers(0, 1, BufferID.GetAddressOf());
	}
	void Unbind() const override
	{
		CurrentGraphicsContext::Context()->VSSetConstantBuffers(0, 1, nullptr);
	}
};

template<typename T>
class PSConstantBuffer : public ConstantBuffer<T>
{
public:
	using ConstantBuffer<T>::ConstantBuffer;
	using ConstantBuffer<T>::BufferID;

	void Bind() const override
	{
		CurrentGraphicsContext::Context()->PSSetConstantBuffers(0, 1, BufferID.GetAddressOf());
	}

	void Unbind() const override
	{
		CurrentGraphicsContext::Context()->PSSetConstantBuffers(0, 1, nullptr);
	}
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