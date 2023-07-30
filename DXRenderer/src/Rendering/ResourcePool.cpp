#include "ResourcePool.h"

void Pool::Add(SharedPtr<Shader> shader)
{
	Get().Shaders.Add(std::move(shader));
}

void Pool::Add(SharedPtr<BufferBase> buffer)
{
	Get().Buffers.Add(std::move(buffer));
}

SharedPtr<Shader> Pool::GetShader(const std::string& id)
{
	return Get().Shaders.Get(id);
}

SharedPtr<BufferBase> Pool::GetBuffer(const std::string& id)
{
	return Get().Buffers.Get(id);
}

Pool& Pool::Get()
{
	static Pool singleton;
	return singleton;
}
