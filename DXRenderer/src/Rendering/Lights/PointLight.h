#pragma once

#include "Rendering\Buffer.h"
#include "Rendering\Actors\Sphere.h"

class PointLight
{
public:
	PointLight();

	void Bind();
	void Draw();
	void GUI();
	void Update();
private:
	Sphere Mesh;
	DirectX::XMFLOAT3 Position;
	Uniform<DirectX::XMFLOAT3> Model;
};