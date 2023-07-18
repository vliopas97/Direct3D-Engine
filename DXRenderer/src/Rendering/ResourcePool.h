#pragma once

#include "Core/Core.h"
#include "Shader.h"
#include "Buffer.h"

class Pool
{
public:
	static void Add(SharedPtr<Shader> shader);
	static void Add(SharedPtr<Buffer> buffer);
	static SharedPtr<Shader> GetShader(const std::string& id);
	static SharedPtr<Buffer> GetBuffer(const std::string& id);
private:
	static Pool& Get();

private:
	ShaderPool Shaders;
	BufferPool Buffers;
};