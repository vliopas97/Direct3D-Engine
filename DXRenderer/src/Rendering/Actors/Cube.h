#pragma once

#include "Actor.h"
#include "Rendering\Utilities.h"

class CubeOutline;

class Cube : public Actor
{
public:
	Cube();
	Cube(const TransformationIntrinsics& intrinsics);

	void Submit(size_t channelsIn) override;
	void Tick(float delta) override;
private:
	void Init();

private:
	DirectX::XMMATRIX ModelViewOutline;
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