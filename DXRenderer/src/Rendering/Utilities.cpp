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

DirectX::XMVECTOR ToEuler(DirectX::XMVECTOR q)
{
	DirectX::XMVECTOR angles;

	// roll (x-axis rotation)	
	double sinr_cosp = 2 * (q.m128_f32[3] * q.m128_f32[0] + q.m128_f32[1] * q.m128_f32[2]);
	double cosr_cosp = 1 - 2 * (q.m128_f32[0] * q.m128_f32[0] + q.m128_f32[1] * q.m128_f32[1]);
	angles.m128_f32[0] = std::atan2(sinr_cosp, cosr_cosp);

	// pitch (y-axis rotation)
	double sinp = std::sqrt(1 + 2 * (q.m128_f32[3] * q.m128_f32[1] - q.m128_f32[0] * q.m128_f32[2]));
	double cosp = std::sqrt(1 - 2 * (q.m128_f32[3] * q.m128_f32[1] - q.m128_f32[0] * q.m128_f32[2]));
	angles.m128_f32[1] = 2 * std::atan2(sinp, cosp) - DirectX::XM_PI / 2;

	// yaw (z-axis rotation)
	double siny_cosp = 2 * (q.m128_f32[3] * q.m128_f32[2] + q.m128_f32[0] * q.m128_f32[1]);
	double cosy_cosp = 1 - 2 * (q.m128_f32[1] * q.m128_f32[1] + q.m128_f32[2] * q.m128_f32[2]);
	angles.m128_f32[2] = std::atan2(siny_cosp, cosy_cosp);

	return angles;
}
