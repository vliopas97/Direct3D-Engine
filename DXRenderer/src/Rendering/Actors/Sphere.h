#pragma once

#include "Actor.h"

class Sphere : public Actor
{
public:
	Sphere();
	Sphere(const TransformationIntrinsics& intrinsics);

	virtual void Update() override;

private:
	void Init();
};