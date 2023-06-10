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
	Components.Bind();
	CurrentGraphicsContext::Context()->DrawIndexed(GetIndexBuffer()->GetCount(), 0, 0);
}

DirectX::XMMATRIX Actor::GetTransform() const
{
	return Transform;
}

void Actor::Update()
{
}

inline const IndexBuffer* Actor::GetIndexBuffer() const
{
	const IndexBuffer* ptr = GetTypeBuffers().GetIndexBuffer();
	ASSERT(ptr);
	return ptr;
}
