#pragma once

#include "Core/Core.h"
#include "Shader.h"
#include "Buffer.h"

class Pool
{
public:
	static void Add(SharedPtr<Shader> shader);
	static void Add(SharedPtr<BufferBase> buffer);
	static SharedPtr<Shader> GetShader(const std::string& id);
	static SharedPtr<BufferBase> GetBuffer(const std::string& id);
private:
	static Pool& Get();

private:
	ShaderPool Shaders;
	BufferPool Buffers;
};