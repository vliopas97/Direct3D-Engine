#include "Actor.h"

#include "Rendering\Graphics.h"
#include "Rendering\CurrentGraphicsContext.h"

#include <vector>


TransformationMatrix::TransformationMatrix()
	: Intrinsics(), Matrix(DirectX::XMMatrixIdentity())
{
}

inline TransformationMatrix::TransformationMatrix(const DirectX::XMMATRIX& matrix)
	: Intrinsics(), Matrix(matrix)
{
}

inline TransformationMatrix::TransformationMatrix(const TransformationIntrinsics& intrinsics)
	: Intrinsics(intrinsics), Matrix(DirectX::XMMatrixIdentity())
{
	Update();
}

inline void TransformationMatrix::Update()
{
	Matrix = DirectX::XMMatrixScaling(Sx, Sy, Sz) *
		DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(Pitch),
											  DirectX::XMConvertToRadians(Yaw),
											  DirectX::XMConvertToRadians(Roll)) *
		DirectX::XMMatrixTranslation(X, Y, Z);
}

Actor::Actor()
	: Transform()
{
}

Actor::Actor(const TransformationIntrinsics& intrinsics)
	: Transform(intrinsics)
{
}

void Actor::Draw()
{
	GetTypeShaders().Bind();
	GetTypeBuffers().Bind();
	InstanceBuffers.Bind();
	CurrentGraphicsContext::Context()->DrawIndexed(GetTypeBuffers().GetIndexBuffer()->GetCount(), 0, 0);
}

DirectX::XMMATRIX Actor::GetTransform() const
{
	return Transform;
}

void Actor::Update()
{
}
