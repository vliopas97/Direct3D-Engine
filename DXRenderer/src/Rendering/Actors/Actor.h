#pragma once

#include "Core\Core.h"

#include "Rendering\Buffer.h"
#include "Rendering\Shader.h"

#include <d3d11.h>
#include <DirectXMath.h>

class Buffer;
class BufferGroup;
struct Shader;
struct ShaderGroup;

struct TransformationIntrinsics
{
	TransformationIntrinsics() = default;
	TransformationIntrinsics(const TransformationIntrinsics&) = default;

	float Sx = 1.0f;
	float Sy = 1.0f;
	float Sz = 1.0f;
	float Roll = 0.0f;
	float Pitch = 0.0f;
	float Yaw = 0.0f;
	float X = 0.0f;
	float Y = 0.0f;
	float Z = 0.0f;
};

struct TransformationMatrix
{
	TransformationMatrix();
	TransformationMatrix(const DirectX::XMMATRIX& matrix);
	TransformationMatrix(const TransformationIntrinsics& intrinsics);

	operator DirectX::XMMATRIX() const { return Matrix; }

	inline const DirectX::XMMATRIX& GetMatrix() const { return Matrix; }

	void Update();

	struct
	{
		union
		{
			DirectX::XMVECTOR Scale;
			struct
			{
				float Sx;
				float Sy;
				float Sz;
			};
		};

		union
		{
			DirectX::XMVECTOR Rotation;
			struct
			{
				float Roll;
				float Pitch;
				float Yaw;
			};
		};

		union
		{
			DirectX::XMVECTOR translation;
			struct
			{
				float X;
				float Y;
				float Z;
			};
		};
	};
private:
	DirectX::XMMATRIX Matrix;
};

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
