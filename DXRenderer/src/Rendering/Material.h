#pragma once

#include "Buffer.h"
#include "Component.h"

#include <DirectXMath.h>

class Material : public Component
{
public:

	Material(uint32_t slot = 0);

	void Bind() const override;

public:

	struct MaterialProperties
	{
		alignas(16) DirectX::XMFLOAT3 Color{1.0f, 1.0f, 1.0f};
		float specularIntensity{ 1.0f };
		float Shininess{ 12.0f };
		BOOL NormalMapEnabled = TRUE;
	};
	
	union
	{
		MaterialProperties Properties;
		struct
		{
			DirectX::XMFLOAT3 Color;
		};
	};
private:
	Uniform<MaterialProperties> PropertiesUniform;
};