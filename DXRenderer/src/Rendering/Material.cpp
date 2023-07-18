#include "Material.h"

Material::Material(uint32_t slot)
	:Properties(), 
	PropertiesUniform(
		MakeUnique<PSConstantBuffer<MaterialProperties>>(std::string(typeid(this).name()) + "properties",
														 Properties,
														 slot),
		Properties)
{
}

void Material::Bind() const
{
	PropertiesUniform.Bind();
}