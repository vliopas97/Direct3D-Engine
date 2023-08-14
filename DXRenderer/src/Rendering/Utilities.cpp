#include "Utilities.h"
#include "RenderQueue.h"

TransformationMatrix::TransformationMatrix()
	: Matrix(DirectX::XMMatrixIdentity()), Scale( 1.0f, 1.0f, 1.0f), Rotation(0.0f, 0.0f, 0.0f), 
	Translation(0.0f, 0.0f, 0.0f)
{
}

TransformationMatrix::TransformationMatrix(const TransformationIntrinsics& intrinsics)
	: Matrix(DirectX::XMMatrixIdentity())
{
	Sx = intrinsics.Sx;
	Sy = intrinsics.Sy;
	Sz = intrinsics.Sz;
	Roll = intrinsics.Roll;
	Pitch = intrinsics.Pitch;
	Yaw = intrinsics.Yaw;
	X = intrinsics.X;
	Y = intrinsics.Y;
	Z = intrinsics.Z;
	Update();
}

void TransformationMatrix::Update()
{
	Matrix = DirectX::XMMatrixScaling(Sx, Sy, Sz) *
		DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(-Pitch),
											  DirectX::XMConvertToRadians(-Yaw),
											  DirectX::XMConvertToRadians(Roll)) *
		DirectX::XMMatrixTranslation(-X, -Y, Z);
}

GPUObjectBase::GPUObjectBase(GPUObjectBase&& other) noexcept
{
	Buffers = std::move(other.Buffers);
	Shaders = std::move(other.Shaders);
	Components = std::move(other.Components);
}

void GPUObjectBase::Add(SharedPtr<Shader> shader)
{
	const auto id = shader->GetID();

	Pool::Add(shader);
	Shaders.Add(Pool::GetShader(id));
}

void GPUObjectBase::Add(SharedPtr<BufferBase> buffer)
{
	const auto id = buffer->GetID();

	Pool::Add(buffer);
	Buffers.Add(Pool::GetBuffer(id));
}

void GPUObjectBase::Add(UniquePtr<Component> component)
{
	Components.Add(std::move(component));
}

void GPUObjectBase::Bind() const
{
	Buffers.Bind();
	Shaders.Bind();
	Components.Bind();
}

const IndexBuffer* GPUObjectBase::GetIndexBuffer() const
{
	const IndexBuffer* ptr = Buffers.GetIndexBuffer();
	ASSERT(ptr);
	return ptr;
}

GPUObject::GPUObject(GPUObject&& other) noexcept
	:GPUObjectBase(std::move(other))
{
	Techniques = std::move(other.Techniques);
}

void GPUObject::Add(Technique&& technique)
{
	Techniques.emplace_back(MakeUnique<Technique>(std::move(technique)));
}
