#pragma once

#include "Core\Core.h"
#include "Rendering\Buffer.h"
#include "Rendering\Component.h"
#include "Rendering\ResourcePool.h"
#include "Rendering\Shader.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <numbers>
#include <vector>
#include <sstream>

constexpr std::array<float, 4> backgroundColor = { 1.0f, 0.5f, 0.0f, 0.0f };

template<typename T>
constexpr T gauss(T x, T sigma) noexcept
{
	const auto ss = std::pow(sigma, 2.0f);
	return ((T)1.0 / std::pow((T)2.0 * (T)std::numbers::pi_v<T> * ss, 2.0f)) * exp(-std::pow(x, 2.0f) / ((T)2.0 * ss));
}

inline std::vector<std::string> SplitString(const std::string& input, const std::string& delim)
{
	std::vector<std::string> output;
	std::istringstream stream(input);

	std::string part;
	while (std::getline(stream, part, '.'))
		output.push_back(part);

	return output;
}

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

	inline DirectX::XMMATRIX& GetMatrix() 
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

class Technique;

class GPUObjectBase
{
public:
	GPUObjectBase() = default;
	virtual ~GPUObjectBase() = default;
	GPUObjectBase(GPUObjectBase&& other) noexcept;

	virtual void Tick(float delta) {}
	void Bind() const;
	const IndexBuffer* GetIndexBuffer() const;

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

class GPUObject : public GPUObjectBase
{
public:
	using GPUObjectBase::Add;
	GPUObject() = default;
	virtual ~GPUObject() = default;
	GPUObject(GPUObject&& other) noexcept;
	void Add(Technique&& technique);
	virtual void LinkTechniques();

protected:
	std::vector<UniquePtr<Technique>> Techniques;
};