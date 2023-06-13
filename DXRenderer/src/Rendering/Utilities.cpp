#include "Utilities.h"

TransformationMatrix::TransformationMatrix()
	: Matrix(DirectX::XMMatrixIdentity()), Scale(DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f)), Rotation(DirectX::XMVectorZero()), translation(DirectX::XMVectorZero())
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