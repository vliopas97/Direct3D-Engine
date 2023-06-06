#pragma once

#include "Core\Core.h"

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

	operator DirectX::XMMATRIX() const
	{
		return Matrix;
	}

	void Update();

	union
	{
		TransformationIntrinsics Intrinsics;
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

	void AddBuffer(UniquePtr<Buffer> buffer);
	void AddShader(UniquePtr<Shader> shader);

	void Draw();

	DirectX::XMMATRIX GetTransform() const;

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
	UniquePtr<BufferGroup> Buffers;
	UniquePtr<ShaderGroup> Shaders;
};

class Cube : public Actor
{
public:
	Cube();
	Cube(const TransformationIntrinsics& intrinsics);

private:
	void Init();
};