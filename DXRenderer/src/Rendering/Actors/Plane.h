#pragma once

#include "Actor.h"
#include "Primitives.h"

#include <imgui.h>

class Plane : public Actor
{
public:
	Plane();
	Plane(const TransformationIntrinsics& intrinsics);

	virtual void GUI() override;
private:
	void Init();
};