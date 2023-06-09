#pragma once

#include "Core\Core.h"

#include "Rendering\Buffer.h"
#include "Rendering\Shader.h"
#include "Rendering\Utilities.h"

#include <d3d11.h>
#include <DirectXMath.h>

class Actor
{
public:
	Actor();
	Actor(const TransformationIntrinsics& intrinsics);

	virtual ~Actor() = default;
	Actor(const Actor&) = delete;
	Actor& operator=(const Actor&) = delete;

	void Draw();

	DirectX::XMMATRIX GetTransform() const;

	virtual void Update();

protected:
	virtual const BufferGroup& GetTypeBuffers() const = 0;
	virtual const ShaderGroup& GetTypeShaders() const = 0;

public:
	union
	{
		TransformationMatrix Transform;
		struct
		{
			float Sx;
			float Sy;
			float Sz;
			float Roll;
			float Pitch;
			float Yaw;
			float X;
			float Y;
			float Z;
		};
	};

protected:
	BufferGroup InstanceBuffers;
};

template<typename T>
class ActorBase : public Actor
{
	using Actor::Actor;
public:
	bool IsInitialized() const
	{
		return Shaders.Size() && Buffers.Size();
	}

	void AddBuffer(UniquePtr<Buffer> buffer)
	{
		Buffers.Add(std::move(buffer));
	}

	void AddShader(UniquePtr<Shader> shader)
	{
		Shaders.Add(std::move(shader));
	}

protected:
	virtual void InitializeType() = 0;

	const BufferGroup& GetTypeBuffers() const override
	{
		return Buffers;
	}

	const ShaderGroup& GetTypeShaders() const override
	{
		return Shaders;
	}

private:
	static BufferGroup Buffers;
	static ShaderGroup Shaders;
};

template<typename T>
BufferGroup ActorBase<T>::Buffers;

template<typename T>
ShaderGroup ActorBase<T>::Shaders;
