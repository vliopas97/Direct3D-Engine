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
	for (auto& t : Techniques)
		t->Submit(*this);
}

DirectX::XMMATRIX Actor::GetTransform() const
{
	return Transform;
}

void Actor::Tick(float delta)
{
	Transform.Update();
	ModelView = Transform.GetMatrix() * CurrentGraphicsContext::GraphicsInfo->GetCamera().GetView();
}

