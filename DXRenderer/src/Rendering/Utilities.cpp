#include "Utilities.h"

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

void GPUObject::Add(SharedPtr<Shader> shader)
{
	const auto id = shader->GetID();

	Pool::Add(shader);
	Shaders.Add(Pool::GetShader(id));
}

void GPUObject::Add(SharedPtr<BufferBase> buffer)
{
	const auto id = buffer->GetID();

	Pool::Add(buffer);
	Buffers.Add(Pool::GetBuffer(id));
}

void GPUObject::Add(UniquePtr<Component> component)
{
	Components.Add(std::move(component));
}

void GPUObject::Bind() const
{
	Buffers.Bind();
	Shaders.Bind();
	Components.Bind();
}

const IndexBuffer* GPUObject::GetIndexBuffer() const
{
	const IndexBuffer* ptr = Buffers.GetIndexBuffer();
	ASSERT(ptr);
	return ptr;
}
