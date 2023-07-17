#pragma once

#include "Core/Core.h"
#include "Shader.h"

class Pool
{
public:
	static void Add(SharedPtr<Shader> shader);

	static SharedPtr<Shader> GetShader(const std::string& id);

private:
	static Pool& Get();

private:
	ShaderPool Shaders;
};