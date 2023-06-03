#pragma once

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

class BufferBase
{
public:
	virtual void Unbind() const	= 0;
	virtual ~BufferBase() = default;

protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> BufferID;
};

class Buffer : public BufferBase
{
public:
	virtual void Bind() const = 0;
	virtual ~Buffer() = default;
};

class VertexBuffer : public Buffer
{
public:
	VertexBuffer(const VertexElement* vertices, int size);

	void Bind() const override;
	void Unbind() const override;

	void SetLayout(const BufferLayout& layout);

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

enum ShaderToBind
{
	Vertex,
	Pixel
};

template<typename T>
class ConstantBuffer : public BufferBase
{
public:
	ConstantBuffer(const T& matrix);
	void Bind(ShaderToBind shader) const;
	void Unbind() const override;
private:
	T Matrix;
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