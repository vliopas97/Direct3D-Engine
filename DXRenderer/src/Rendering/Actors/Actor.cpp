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
