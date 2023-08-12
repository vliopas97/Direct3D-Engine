#include "Buffer.h"
#include "CurrentGraphicsContext.h"
#include "Graphics.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>

LayoutElement::LayoutElement(const std::string& name, DataType type)
	:Name(name), Type(type), Size(CalcSize(type)), Offset(0)
{
}

LayoutElement::LayoutElement(ElementType type)
	:Name(ResolveNameFromType(type)), Type(ResolveDataType(type)), Size(CalcSize(type)), Offset(0)
{}

const char* LayoutElement::ResolveNameFromType(ElementType type)
{
	switch (type)
	{
	case ElementType::Position2:
	case ElementType::Position3: return "Position";
	case ElementType::Color3:
	case ElementType::Color4: return "Color";
	case ElementType::Normal: return "Normal";
	case ElementType::Tangent: return "Tangent";
	case ElementType::Bitangent: return "Bitangent";
	case ElementType::TexCoords: return "TexCoords";
	}
}

DXGI_FORMAT LayoutElement::DataTypeToDXGI(DataType type)
{
	switch (type)
	{
	case DataType::UChar2Norm: return DXGI_FORMAT_R8G8_UNORM;
	case DataType::UChar4Norm: return DXGI_FORMAT_R8G8B8A8_UNORM;
	case DataType::UChar2: return DXGI_FORMAT_R8G8_UINT;
	case DataType::UChar4: return DXGI_FORMAT_R8G8B8A8_UINT;
	case DataType::Float: return DXGI_FORMAT_R32_FLOAT;
	case DataType::Float2: return DXGI_FORMAT_R32G32_FLOAT;
	case DataType::Float3: return DXGI_FORMAT_R32G32B32_FLOAT;
	case DataType::Float4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case DataType::Int: return  DXGI_FORMAT_R32_SINT;
	case DataType::Int2: return DXGI_FORMAT_R32G32_SINT;
	case DataType::Int3: return DXGI_FORMAT_R32G32B32_SINT;
	case DataType::Int4: return DXGI_FORMAT_R32G32B32A32_SINT;
	}
}

LayoutElement::DataType LayoutElement::ResolveDataType(ElementType type)
{
	switch (type)
	{
	case ElementType::Position2: return DataType::Float2;
	case ElementType::Position3: return DataType::Float3;
	case ElementType::Color3: return DataType::Float3;
	case ElementType::Color4: return DataType::Float4;
	case ElementType::Normal: return DataType::Float3;
	case ElementType::Tangent: return DataType::Float3;
	case ElementType::Bitangent: return DataType::Float3;
	case ElementType::TexCoords: return DataType::Float2;
	}
}

uint32_t LayoutElement::CalcSize(DataType type)
{
	switch (type)
	{
	case DataType::UChar2:
	case DataType::UChar2Norm: return sizeof(unsigned char) * 2;
	case DataType::UChar4:
	case DataType::UChar4Norm: return sizeof(unsigned char) * 4;
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

uint32_t LayoutElement::CalcSize(ElementType type)
{
	switch (type)
	{
	case ElementType::Position2: return sizeof(float) * 2;
	case ElementType::Position3: return sizeof(float) * 3;
	case ElementType::Color3: return sizeof(float) * 3;
	case ElementType::Color4: return sizeof(float) * 4;
	case ElementType::Normal: return sizeof(float) * 3;
	case ElementType::Tangent: return sizeof(float) * 3;
	case ElementType::Bitangent: return sizeof(float) * 3;
	case ElementType::TexCoords: return sizeof(float) * 2;
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

const LayoutElement& BufferLayout::operator[](size_t i) const
{
	return Elements[i];
}

inline VertexBuffer::VertexBuffer(const std::string& tag,
								  const BufferLayout& layout)
	:Buffer(tag), Layout(layout), Topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
{
}

void VertexBuffer::Bind() const
{
	UINT stride = Layout.GetStride();
	UINT offset = 0;
	CurrentGraphicsContext::Context()->IASetVertexBuffers(0, 1, BufferID.GetAddressOf(), &stride, &offset);
	CurrentGraphicsContext::Context()->IASetPrimitiveTopology(Topology);
}

void VertexBuffer::Unbind() const
{
	CurrentGraphicsContext::Context()->IASetVertexBuffers(0, 0, nullptr, 0, 0);
}

std::string VertexBuffer::GetID() const
{
	return std::string(typeid(VertexBuffer).name()) + "#" + Tag;
}

BufferType VertexBuffer::GetType() const
{
	return Type;
}

BufferLayout VertexBuffer::GetLayout() const { return Layout; }

VertexBufferBuilder::VertexBufferBuilder(const std::string& tag,
										 BufferLayout&& layout, const Microsoft::WRL::ComPtr<ID3DBlob>& blob)
	: Object(tag, std::move(layout))
{
	//std::vector<D3D11_INPUT_ELEMENT_DESC> desc;
	//desc.reserve(Object.Layout.GetElementsSize());

	//for (size_t i = 0; i < Object.Layout.GetElementsSize(); i++)
	//{
	//	const auto& element = Object.Layout[i];
	//	desc.emplace_back(element.Name.c_str(), 0, LayoutElement::DataTypeToDXGI(element.Type),
	//		0, element.Offset, D3D11_INPUT_PER_VERTEX_DATA, 0);
	//}

	//CurrentGraphicsContext::Device()->CreateInputLayout(desc.data(), (UINT)std::size(desc), Object.Blob->GetBufferPointer(),
	//	Object.Blob->GetBufferSize(), &Object.InputLayout);
}

UniquePtr<VertexBuffer> VertexBufferBuilder::Release()
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.ByteWidth = Vertices.size();
	vertexBufferDesc.StructureByteStride = Object.Layout.GetStride();

	D3D11_SUBRESOURCE_DATA subResourceData;
	subResourceData.pSysMem = Vertices.data();

	CurrentGraphicsContext::Device()->CreateBuffer(&vertexBufferDesc, &subResourceData, &Object.BufferID);

	return MakeUnique<VertexBuffer>(std::move(Object));
}

Vertex VertexBufferBuilder::Back()
{
	ASSERT(Vertices.size() != 0);
	return Vertex{ Vertices.data() + Vertices.size() - Object.Layout.GetStride(), Object.Layout };
}

Vertex VertexBufferBuilder::Front()
{
	ASSERT(Vertices.size() != 0);
	return Vertex{ Vertices.data(), Object.Layout };
}

IndexBuffer::IndexBuffer(const std::string& tag, const std::vector<unsigned short>& indices)
	:Buffer(tag), Count(static_cast<UINT>(indices.size()))
{
	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.ByteWidth = sizeof(unsigned short) * indices.size();
	indexBufferDesc.StructureByteStride = sizeof(unsigned short);

	D3D11_SUBRESOURCE_DATA subResourceData;
	subResourceData.pSysMem = indices.data();
	CurrentGraphicsContext::Device()->CreateBuffer(&indexBufferDesc, &subResourceData, &BufferID);
}

void IndexBuffer::Bind() const
{
	CurrentGraphicsContext::Context()->IASetIndexBuffer(BufferID.Get(), DXGI_FORMAT_R16_UINT, 0);
}

void IndexBuffer::Unbind() const
{
	CurrentGraphicsContext::Context()->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
}

BufferType IndexBuffer::GetType() const
{
	return Type;
}

UINT IndexBuffer::GetCount() const
{
	return Count;
}

std::string IndexBuffer::GetID() const
{
	return std::string(typeid(IndexBuffer).name()) + "#" + Tag;
}

DepthBuffer::DepthBuffer(Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& dSV)
{
	//CreateDepthStencilState();
	CreateDepthStencilTexture();
	CreateDepthStencilView(dSV);
}

void DepthBuffer::Bind() const
{
	CurrentGraphicsContext::Context()->OMSetDepthStencilState(depthStencilState.Get(), 1);
}

void DepthBuffer::Unbind() const
{
	CurrentGraphicsContext::Context()->OMSetDepthStencilState(nullptr, 1);
}

void DepthBuffer::CreateDepthStencilState()
{
	D3D11_DEPTH_STENCIL_DESC depthBufferDesc{};
	depthBufferDesc.DepthEnable = TRUE;
	depthBufferDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthBufferDesc.DepthFunc = D3D11_COMPARISON_LESS;

	CurrentGraphicsContext::Device()->CreateDepthStencilState(&depthBufferDesc, &depthStencilState);
}

void DepthBuffer::CreateDepthStencilTexture()
{
	D3D11_TEXTURE2D_DESC descDepth{};
	descDepth.Width = 1904;
	descDepth.Height = 1041;
	descDepth.MipLevels = 1u;
	descDepth.ArraySize = 1u;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1u;
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	CurrentGraphicsContext::Device()->CreateTexture2D(&descDepth, nullptr, &depthStencil);
}

void DepthBuffer::CreateDepthStencilView(Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& dSV) const
{
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0u;

	CurrentGraphicsContext::Device()->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, &dSV);
}

void BufferGroup::Add(SharedPtr<BufferBase> buffer)
{
	auto it = std::find_if(Buffers.begin(), Buffers.end(), [&buffer](const SharedPtr<BufferBase>& element)
		{
			return typeid(*element) == typeid(buffer);
		});
	if (it != Buffers.end())
		return;

	Buffers.emplace_back(std::move(buffer));
}

const IndexBuffer* BufferGroup::GetIndexBuffer() const
{
	auto it = std::find_if(Buffers.begin(), Buffers.end(), [](const SharedPtr<BufferBase>& element)
		{
			return dynamic_cast<IndexBuffer*>(element.get());
		});

	if (it != Buffers.end())
		return dynamic_cast<IndexBuffer*>(it->get());

	return nullptr;
}

void BufferGroup::Bind() const
{
	for (const auto& buffer : Buffers)
		buffer->Bind();
}

void BufferGroup::Unbind() const
{
	for (const auto& buffer : Buffers)
		buffer->Unbind();
}

inline Vertex::Vertex(char* ptr, const BufferLayout& layout)
	: Ptr(ptr), Layout(layout)
{
	ASSERT(ptr);
}

inline Buffer::Buffer(const std::string& tag)
	: Tag(tag)
{}

void BufferPool::Add(SharedPtr<BufferBase> buffer)
{
	if (Get(buffer->GetID()))
		return;

	Buffers[buffer->GetID()] = std::move(buffer);
}

SharedPtr<BufferBase> BufferPool::Get(const std::string& id)
{
	auto it = Buffers.find(id);
	if (it == Buffers.end())
		return {};
	else
		return it->second;
}

InputLayout::InputLayout(const std::string& tag, const BufferLayout& layout, const Microsoft::WRL::ComPtr<ID3DBlob>& blob)
	:Tag(tag)
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> desc{};
	desc.reserve(layout.GetElementsSize());

	for (auto& element : layout.Elements)
	{
		desc.emplace_back(element.Name.c_str(), 0, LayoutElement::DataTypeToDXGI(element.Type),
						  0, element.Offset, D3D11_INPUT_PER_VERTEX_DATA, 0);
	}

	GRAPHICS_ASSERT(CurrentGraphicsContext::Device()->CreateInputLayout(desc.data(), (UINT)std::size(desc), blob->GetBufferPointer(), blob->GetBufferSize(), &BufferID));
}

void InputLayout::Bind() const
{
	CurrentGraphicsContext::Context()->IASetInputLayout(BufferID.Get());
}

void InputLayout::Unbind() const
{
	CurrentGraphicsContext::Context()->IASetInputLayout(nullptr);
}

std::string InputLayout::GetID() const
{
	return std::string(typeid(InputLayout).name()) + "#" + Tag;
}
