#include "Material.h"

Material::Material(uint32_t slot)
	:Properties(),
	PropertiesUniform(UniformPS<MaterialProperties>(std::string(typeid(this).name()) + "properties", Properties, slot))
{
}

void Material::Bind() const
{
	PropertiesUniform.Bind();
}