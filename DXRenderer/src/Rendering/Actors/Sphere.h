#pragma once

#include "Actor.h"

class Sphere : public Actor
{
public:
	Sphere();
	Sphere(const TransformationIntrinsics& intrinsics);

	virtual void Tick(float delta) override;

private:
	void Init();

private:
	DirectX::XMMATRIX viewProjection;
};