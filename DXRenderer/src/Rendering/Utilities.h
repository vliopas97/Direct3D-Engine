#pragma once

#include "Core\Core.h"
#include "Rendering\Buffer.h"
#include "Rendering\Component.h"
#include "Rendering\ResourcePool.h"
#include "Rendering\Shader.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

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
	TransformationMatrix(const TransformationIntrinsics& intrinsics);

	operator DirectX::XMMATRIX() const
	{
		return Matrix;
	}

	inline const DirectX::XMMATRIX& GetMatrix() const
	{
		return Matrix;
	}

	void Update();

	struct
	{
		union
		{
			DirectX::XMFLOAT3 Scale;
			struct
			{
				float Sx;
				float Sy;
				float Sz;
			};
		};

		union
		{
			DirectX::XMFLOAT3 Rotation;
			struct
			{
				float Roll;
				float Pitch;
				float Yaw;
			};
		};

		union
		{
			DirectX::XMFLOAT3 Translation;
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

namespace Primitives
{
	struct VertexElement;
}

template<typename Vertex,typename = std::enable_if_t<std::is_base_of_v<Primitives::VertexElement, Vertex>>>
struct IndexedVertices
{
	IndexedVertices() = default;
	IndexedVertices(const std::vector<Vertex>& vertices, const std::vector<unsigned short> indices)
		: Vertices(std::move(vertices)), Indices(std::move(indices))
	{
	}

	void Transform(const TransformationMatrix& matrix)
	{
		Transform(matrix.GetMatrix());
	}

	void Transform(const DirectX::XMMATRIX& matrix)
	{
		for (auto& v : Vertices)
		{
			const auto& position = DirectX::XMLoadFloat3(&v.Position);
			DirectX::XMStoreFloat3(
				&v.Position,
				DirectX::XMVector3Transform(position, matrix));
		}
	}

public:
	std::vector<Vertex> Vertices;
	std::vector<unsigned short> Indices;
};

class GPUObject
{
protected:
	GPUObject() = default;
	virtual ~GPUObject() = default;

	void Add(SharedPtr<Shader> shader);
	void Add(SharedPtr<BufferBase> buffer);
	void Add(UniquePtr<Component> component);

	template <class T, typename... Args>
	requires std::is_base_of_v<Shader, T> || std::is_base_of_v<BufferBase, T> || std::is_base_of_v<Component, T>
	void Add(Args&&... args)
	{
		if constexpr (std::is_base_of_v<BufferBase, T>)
			Add(MakeShared<T>(std::forward<Args>(args)...));
		else
			if constexpr (std::is_base_of_v<Shader, T>)
				Add(MakeShared<T>(std::forward<Args>(args)...));
			else
				Add(MakeUnique<T>(std::forward<Args>(args)...));
	}

protected:
	BufferGroup Buffers;
	ShaderGroup Shaders;
	ComponentGroup Components;
};