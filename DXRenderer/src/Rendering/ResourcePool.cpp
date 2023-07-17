#include "ResourcePool.h"

void Pool::Add(SharedPtr<Shader> shader)
{
	Get().Shaders.Add(std::move(shader));
}

SharedPtr<Shader> Pool::GetShader(const std::string& id)
{
	return Get().Shaders.Get(id);
}

Pool& Pool::Get()
{
	static Pool singleton;
	return singleton;
}
