#pragma once

#include "Actor.h"

class Sphere : public ActorBase<Sphere>
{
public:
	Sphere();
	Sphere(const TransformationIntrinsics& intrinsics);

	virtual void Update() override;

protected:
	void InitializeType() override;
private:
	void Init();
};