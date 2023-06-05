#pragma once
#include "Core\Core.h"
#include "Rendering\Buffer.h"
#include "Rendering\Shader.h"


class Actor
{
public:
	virtual ~Actor() = default;

	void AddBuffer(UniquePtr<Buffer> buffer);
	void AddShader(UniquePtr<Shader> shader);

	void Draw();
protected:
	BufferGroup Buffers;
	ShaderGroup Shaders;
};

class Cube : public Actor
{
public:
	Cube();
};