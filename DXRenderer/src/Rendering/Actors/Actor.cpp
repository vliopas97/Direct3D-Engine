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
	GetTypeBuffers().Bind();
	InstanceBuffers.Bind();
	Shaders.Bind();
	Components.Bind();
	CurrentGraphicsContext::Context()->DrawIndexed(GetIndexBuffer()->GetCount(), 0, 0);
}

DirectX::XMMATRIX Actor::GetTransform() const
{
	return Transform;
}

void Actor::Update()
{
	Transform.Update();
}

void Actor::Add(SharedPtr<Shader> shader)
{
	auto id = shader->GetID();

	Pool::Add(shader);
	Shaders.Add(Pool::GetShader(id));
}

inline const IndexBuffer* Actor::GetIndexBuffer() const
{
	const IndexBuffer* ptr = GetTypeBuffers().GetIndexBuffer();
	ASSERT(ptr);
	return ptr;
}
