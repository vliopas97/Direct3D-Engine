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
		case DataType::UCharNorm:  return DXGI_FORMAT_R8_UNORM;
		case DataType::UChar2Norm: return DXGI_FORMAT_R8G8_UNORM;
		case DataType::UChar4Norm: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case DataType::UChar:  return DXGI_FORMAT_R8_UINT;
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

uint32_t LayoutElement::CalcSize(DataType type)
{
	switch (type)
	{
		case DataType::UChar:
		case DataType::UCharNorm: return sizeof(unsigned char);
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

void VertexBuffer::Bind() const
{
	UINT stride = Layout.GetStride();
	UINT offset = 0;
	CurrentGraphicsContext::Context()->IASetVertexBuffers(0, 1, BufferID.GetAddressOf(), &stride, &offset);
	CurrentGraphicsContext::Context()->IASetPrimitiveTopology(Topology);
	BindLayout();
}

void VertexBuffer::Unbind() const
{
	CurrentGraphicsContext::Context()->IASetVertexBuffers(0, 0, nullptr, 0, 0);
}

void VertexBuffer::SetLayout(const BufferLayout& layout)
{
	Layout = layout;
}

VertexBuffer& VertexBuffer::AddLayoutElement(LayoutElement element)
{
	Layout.AddElement(element);
	return *this;
}

void VertexBuffer::BindLayout() const
{
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	std::vector<D3D11_INPUT_ELEMENT_DESC> desc;
	desc.reserve(Layout.GetElementsSize());

	for (auto& element : Layout.Elements)
	{
		desc.emplace_back(element.Name.c_str(), 0, LayoutElement::DataTypeToDXGI(element.Type),
						  0, element.Offset, D3D11_INPUT_PER_VERTEX_DATA, 0);
	}

	CurrentGraphicsContext::Device()->CreateInputLayout(desc.data(), (UINT)std::size(desc), Blob->GetBufferPointer(),
								   Blob->GetBufferSize(), &inputLayout);

	CurrentGraphicsContext::Context()->IASetInputLayout(inputLayout.Get());
}

BufferType VertexBuffer::GetType() const
{
	return Type;
}

IndexBuffer::IndexBuffer(const std::vector<unsigned short>& indices)
	:Count(static_cast<UINT>(indices.size()))
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

DepthBuffer::DepthBuffer(Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& dSV)
{
	CreateDepthStencilState();
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
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1u;
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	CurrentGraphicsContext::Device()->CreateTexture2D(&descDepth, nullptr, &depthStencil);
}

void DepthBuffer::CreateDepthStencilView(Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& dSV) const
{
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0u;

	CurrentGraphicsContext::Device()->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, &dSV);
}

void BufferGroup::Add(UniquePtr<Buffer> buffer)
{
	auto it = std::find_if(Buffers.begin(), Buffers.end(), [&buffer](const UniquePtr<Buffer>& element)
						   {
							   return typeid(*element) == typeid(buffer);
						   });
	if (it != Buffers.end())
		return;

	Buffers.emplace_back(std::move(buffer));
}

const IndexBuffer* BufferGroup::GetIndexBuffer() const
{
	auto it = std::find_if(Buffers.begin(), Buffers.end(), [](const UniquePtr<Buffer>& element)
						   {
							   return dynamic_cast<IndexBuffer*>(element.get());
						   });

	if(it != Buffers.end())
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

VertexElement::VertexElement(float x, float y, float z) :
	Position(DirectX::XMFLOAT3(x, y, z))
{
}
