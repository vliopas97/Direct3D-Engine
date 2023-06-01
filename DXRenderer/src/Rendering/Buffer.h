#pragma once

#include <d3d11.h>
#include <string>
#include <vector>
#include <wrl.h>

struct LayoutElement
{
	enum class DataType
	{
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

class VertexBuffer
{
public:
	VertexBuffer(const float* vertices, int size);

	void Bind() const;
	void Unbind() const;

	void SetLayout(const BufferLayout& layout);

	void CreateLayout(const Microsoft::WRL::ComPtr<ID3DBlob>& blob);
private:
	BufferLayout Layout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> Buffer;
};
