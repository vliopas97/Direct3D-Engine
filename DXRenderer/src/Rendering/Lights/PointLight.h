#pragma once

#include "Rendering\Buffer.h"
#include "Rendering\Actors\Sphere.h"
#include "Rendering/Camera.h"

class PointLight
{
public:
	PointLight();

	void Bind();
	void Submit(size_t channelsIn);
	void LinkTechniques();
	void GUI();
	void Tick(float delta);

public:
	struct LightProperties
	{
		alignas(16) DirectX::XMFLOAT3 Position{};
		alignas(16) DirectX::XMFLOAT3 Ambient{0.15f, 0.15f, 0.15f};
		alignas(16) DirectX::XMFLOAT3 Diffuse{1.0f, 1.0f, 1.0f};
		float Intensity{ 1.0f };
		float AttenuationConstant{ 1.0f };
		float AttenuationLinear{ 0.05f };
		float AttenuationQuad{ 0.01f };
	};
	
	union
	{
		LightProperties Properties;
		
		struct
		{
			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT3 Ambient;
			DirectX::XMFLOAT3 Diffuse;
			float Intensity;
			float AttenuationConstant;
			float AttenuationLinear;
			float AttenuationQuad;
		};
	};

private:
	Sphere Mesh;
	UniformPS<LightProperties> Model;
};