#pragma once

#include "Actor.h"

class Cube : public Actor
{
public:
	Cube();
	Cube(const TransformationIntrinsics& intrinsics);

private:
	void Init();
};