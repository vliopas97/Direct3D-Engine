#include "Material.h"

Material::Material(uint32_t slot)
	:Properties(), PropertiesUniform(MakeUnique<PSConstantBuffer<MaterialProperties>>(Properties, slot), Properties)
{
}

void Material::Bind() const
{
	PropertiesUniform.Bind();
}