#pragma once

#include "Core\Core.h"

#include "Rendering\Buffer.h"
#include "Rendering\Shader.h"
#include "Rendering\Utilities.h"
#include "Rendering/ResourcePool.h"
#include "Rendering\Texture.h"

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

	virtual void Draw();
	DirectX::XMMATRIX GetTransform() const;
	virtual void Update();

	void Add(SharedPtr<Buffer> buffer);
	void Add(SharedPtr<Shader> shader);

private:
	virtual const IndexBuffer* GetIndexBuffer() const;

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
	ShaderGroup Shaders;
	BufferGroup Buffers;
	ComponentGroup Components;
};