#include "Buffer.h"
#include "CurrentGraphicsContext.h"
#include "Graphics.h"

LayoutElement::LayoutElement(const std::string& name, DataType type)
	:Name(name), Type(type), Size(CalcSize(type))
{
}

DXGI_FORMAT LayoutElement::DataTypeToDXGI(DataType type)
{
	switch (type)
	{
		case DataType::Float: return DXGI_FORMAT_R32_FLOAT;
		case DataType::Float2: return DXGI_FORMAT_R32G32_FLOAT;
		case DataType::Float3: return DXGI_FORMAT_R32G32B32_FLOAT;
		case DataType::Float4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case DataType::Int: return DXGI_FORMAT_R32_SINT;
		case DataType::Int2: return DXGI_FORMAT_R32G32_SINT;
		case DataType::Int3: return DXGI_FORMAT_R32G32B32_SINT;
		case DataType::Int4: return DXGI_FORMAT_R32G32B32A32_SINT;
	}
}

uint32_t LayoutElement::CalcSize(DataType type)
{
	switch (type)
	{
		case DataType::Float: return sizeof(float);
		case DataType::Float2: return sizeof(float) * 2;
		case DataType::Float3: return sizeof(float) * 3;
		case DataType::Float4: return sizeof(float) * 4;
		case DataType::Int: return  sizeof(int);
		case DataType::Int2: return sizeof(int) * 2;
		case DataType::Int3: return sizeof(int) * 3;
		case DataType::Int4: return sizeof(int) * 4;
	}
}

BufferLayout::BufferLayout(std::initializer_list<LayoutElement> elements)
	: Elements(elements)
{
	uint32_t offset = 0;
	for (auto& element : Elements)
	{
		element.Offset = offset;
		offset += element.Size;
		Stride += element.Size;
	}
}

BufferLayout& BufferLayout::AddElement(LayoutElement element)
{
	uint32_t offset = Elements.empty() ? 0 : Elements.back().Offset + Elements.back().Size;
	element.Offset = offset;
	Stride = element.Offset + element.Size;
	Elements.push_back(element);
	return *this;
}

uint32_t BufferLayout::GetStride() const
{
	return Stride;
}

size_t BufferLayout::GetElementsSize() const
{
	return Elements.size();
}

VertexBuffer::VertexBuffer(const float* vertices, int size)
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.ByteWidth = sizeof(float) * size;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subResourceData;
	subResourceData.pSysMem = vertices;

	CurrentGraphicsContext::GraphicsInfo->GetDevice()->CreateBuffer(&vertexBufferDesc, &subResourceData, &Buffer);
}

void VertexBuffer::Bind() const
{
	UINT stride = Layout.GetStride();
	UINT offset = 0;
	CurrentGraphicsContext::GraphicsInfo->GetContext()->IASetVertexBuffers(0, 1, Buffer.GetAddressOf(), &stride, &offset);
}

void VertexBuffer::Unbind() const
{
	CurrentGraphicsContext::GraphicsInfo->GetContext()->IASetVertexBuffers(0, 0, nullptr, 0, 0);
}

void VertexBuffer::SetLayout(const BufferLayout& layout)
{
	Layout = layout;
}

void VertexBuffer::CreateLayout(const Microsoft::WRL::ComPtr<ID3DBlob>& blob)
{
	Bind();
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	std::vector<D3D11_INPUT_ELEMENT_DESC> desc;
	desc.reserve(Layout.GetElementsSize());

	for (auto& element : Layout.Elements)
	{
		desc.emplace_back(element.Name.c_str(), 0, LayoutElement::DataTypeToDXGI(element.Type),
						  0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0);
	}

	CurrentGraphicsContext::GraphicsInfo->GetDevice()->CreateInputLayout(desc.data(), (UINT)std::size(desc), blob->GetBufferPointer(),
								   blob->GetBufferSize(), &inputLayout);

	//Info.Context->IASetInputLayout(inputLayout.Get());
	CurrentGraphicsContext::GraphicsInfo->GetContext()->IASetInputLayout(inputLayout.Get());
}
