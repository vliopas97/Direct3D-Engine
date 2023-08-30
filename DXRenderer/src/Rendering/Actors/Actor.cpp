#include "Actor.h"

#include "Rendering\Graphics.h"
#include "Rendering\CurrentGraphicsContext.h"

#include <vector>

Actor::Actor()
	: Transform()
{
}

Actor::Actor(const TransformationIntrinsics& intrinsics)
	: Transform(intrinsics)
{
}

void Actor::Submit(size_t channelsIn)
{
	for (auto& t : Techniques)
		t->Submit(*this, channelsIn);
}

DirectX::XMMATRIX Actor::GetTransform() const
{
	return Transform;
}

void Actor::Tick(float delta)
{
	Transform.Update();
	ModelView = Transform.GetMatrix() * CurrentGraphicsContext::GraphicsInfo->GetView();
}


void Actor::SetPosition(const DirectX::XMFLOAT3& position)
{
	X = position.x;
	Y = position.y;
	Z = position.z;
}

void Actor::SetRotation(const DirectX::XMFLOAT3& rotation)
{
	Pitch = DirectX::XMConvertToDegrees(rotation.x);
	Yaw =   DirectX::XMConvertToDegrees(rotation.y);
	Roll =  DirectX::XMConvertToDegrees(rotation.z);
	Transform.Update();
}

void Actor::SetRotationDeg(const DirectX::XMFLOAT3 rotationDegrees)
{
	Pitch = rotationDegrees.x;
	Yaw =   rotationDegrees.y;
	Roll =  rotationDegrees.z;
}


