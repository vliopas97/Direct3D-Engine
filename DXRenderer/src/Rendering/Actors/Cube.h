#pragma once

#include "Actor.h"

class Cube : public ActorBase<Cube>
{
public:
	Cube();
	Cube(const TransformationIntrinsics& intrinsics);

	virtual void Update() override;

protected:
	void InitializeType() override;
private:
	void Init();
};