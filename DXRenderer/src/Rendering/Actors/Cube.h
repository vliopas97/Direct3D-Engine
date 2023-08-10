#pragma once

#include "Actor.h"
#include "Rendering\Utilities.h"

class CubeOutline;

class Cube : public Actor
{
public:
	Cube();
	Cube(const TransformationIntrinsics& intrinsics);

	void Draw() override;
	void Tick(float delta) override;
private:
	void Init();
	void DrawOutline();
private:
	float IsOutlined = true;
	UniquePtr<CubeOutline> Outline;
};

class CubeOutline : public Actor
{
public:
	CubeOutline(const Cube& cube);

	void Tick(float delta) override;
private:
	const Cube& CubeRef;
	DirectX::XMFLOAT4 Color = { 1.0f, 0.4f, 0.4f, 1.0f };
};