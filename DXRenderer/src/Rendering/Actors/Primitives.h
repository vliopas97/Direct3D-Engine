#pragma once

#include "Rendering\Utilities.h"

#include <DirectXMath.h>
#include <numbers>
#include <vector>

namespace Primitives
{
	template<typename Vertex>
	concept IsVertexElement = std::is_base_of_v<VertexElement, Vertex>;

	class Cube
	{
	public:
		template<IsVertexElement Vertex>
		static IndexedVertices<Vertex> Create()
		{
			constexpr float side = 0.5f;

			std::vector<DirectX::XMFLOAT3> positions = {
			{ DirectX::XMFLOAT3(-side, -side, -side) },
			{ DirectX::XMFLOAT3(side, -side, -side) },
			{ DirectX::XMFLOAT3(-side, side, -side) },
			{ DirectX::XMFLOAT3(side, side, -side) },
			{ DirectX::XMFLOAT3(-side, -side, side) },
			{ DirectX::XMFLOAT3(side, -side, side) },
			{ DirectX::XMFLOAT3(-side, side, side) },
			{ DirectX::XMFLOAT3(side, side, side) }
			};

			std::vector<Vertex> vertices(positions.size());
			for (size_t i = 0; i < positions.size(); i++)
				vertices[i].Position = positions[i];

			return { std::move(vertices),
						{
							0,2,1, 2,3,1,
							1,3,5, 3,7,5,
							2,6,3, 3,6,7,
							4,5,7, 4,7,6,
							0,4,2, 2,4,6,
							0,1,4, 1,5,4
						}
			};
		}
	};

	class Cone
	{
	public:
		template<IsVertexElement Vertex>
		static IndexedVertices<Vertex> CreateTesselated(int divisions)
		{
			ASSERT(divisions >= 3);

			const auto base = DirectX::XMVectorSet(1.0f, 0.0f, -1.0f, 0.0f);
			const float arcAngle = 2.0f * std::numbers::pi / divisions;

			std::vector<Vertex> vertices;

			// Base Vertices
			for (int arc = 0; arc < divisions; arc++)
			{
				vertices.emplace_back();
				auto v = DirectX::XMVector3Transform(base, DirectX::XMMatrixRotationZ(arcAngle * arc));
				DirectX::XMStoreFloat3(&vertices.back().Position, v);
			}

			// Central Vertices
			vertices.emplace_back();
			vertices.back().Position = { 0.0f, 0.0f, 1.0f };
			const auto center = (unsigned short)(vertices.size() - 1);

			vertices.emplace_back();
			vertices.back().Position = { 0.0f, 0.0f, -1.0f };
			const auto tip = (unsigned short)(vertices.size() - 1);

			// Indices
			std::vector<unsigned short> indices;
			for (unsigned short arc = 0; arc < divisions; arc++)
			{
				// Base Triangles
				indices.push_back(center);
				indices.push_back((arc + 1) % divisions);
				indices.push_back(arc);

				// Side Triangles
				indices.push_back(arc);
				indices.push_back((arc + 1) % divisions);
				indices.push_back(tip);
			}

			return { std::move(vertices), std::move(indices) };
		}

		template<IsVertexElement Vertex>
		static IndexedVertices<Vertex> Create()
		{
			return CreateTesselated<Vertex>(20);
		}
	};

	class Plane
	{
	public:
		template<IsVertexElement Vertex>
		static IndexedVertices<Vertex> CreateTesselated(int divisionsX, int divisionsY)
		{
			ASSERT(divisionsX >= 1);
			ASSERT(divisionsY >= 1);

			constexpr float width = 2.0f;
			constexpr float height = 2.0f;

			const int numOfVerticesX = divisionsX + 1;
			const int numOfVerticesY = divisionsY + 1;

			std::vector<Vertex> vertices(numOfVerticesX * numOfVerticesY);

			const float sideX = width / 2.0f;
			const float sideY = height / 2.0f;
			const float stepX = width / (float)(divisionsX);
			const float stepY = width / (float)(divisionsY);
			const auto bottomLeft = DirectX::XMVectorSet(-sideX, -sideY, 0.0f, 0.0f);

			for (int y = 0, i = 0; y < numOfVerticesY; y++)
			{
				const float posY = float(y) * stepY;
				for (int x = 0; x < numOfVerticesX; x++, i++)
				{
					const auto v = DirectX::XMVectorAdd(bottomLeft, DirectX::XMVectorSet(float(x) * stepX, posY, 0.0f, 0.0f));
					DirectX::XMStoreFloat3(&vertices[i].Position, v);
				}
			}

			std::vector<unsigned short> indices;
			indices.reserve(std::pow(divisionsX * divisionsY, 2) * 6);

			const auto coordsToIndices = [numOfVerticesX](size_t x, size_t y)
			{
				return (unsigned short)(y * numOfVerticesX + x);
			};

			for (size_t y = 0; y < divisionsY; y++)
			{
				for (size_t x = 0; x < divisionsX; x++)
				{
					const std::array<unsigned short, 4> indexArray = {
						coordsToIndices(x, y), coordsToIndices(x + 1 ,y), coordsToIndices(x, y + 1), coordsToIndices(x + 1, y + 1)
					};
					indices.push_back(indexArray[0]);
					indices.push_back(indexArray[2]);
					indices.push_back(indexArray[1]);
					indices.push_back(indexArray[1]);
					indices.push_back(indexArray[2]);
					indices.push_back(indexArray[3]);
				}
			}

			return { std::move(vertices), std::move(indices) };
		}

		template<IsVertexElement Vertex>
		static IndexedVertices<Vertex> Create()
		{
			return CreateTesselated<Vertex>(1, 1);
		}
	};

	class Prism
	{
	public:
		template<IsVertexElement Vertex>
		static IndexedVertices<Vertex> CreateTesselated(int divisions)
		{
			ASSERT(divisions >= 3);

			const auto base = DirectX::XMVectorSet(1.0f, 0.0f, -1.0f, 0.0f);
			const auto offset = DirectX::XMVectorSet(0.0f, 0.0f, 2.0f, 0.0f);
			const float arcAngle = 2.0f * std::numbers::pi / divisions;

			std::vector<Vertex> vertices;
			vertices.emplace_back();
			vertices.back().Position = { 0.0f, 0.0f, -1.0f };
			const auto centerNear = (unsigned short)(vertices.size() - 1);

			vertices.emplace_back();
			vertices.back().Position = { 0.0f, 0.0f, 1.0f };
			const auto centerFar = (unsigned short)(vertices.size() - 1);

			for (int arc = 0; arc < divisions; arc++)
			{
				vertices.emplace_back();
				auto v = DirectX::XMVector3Transform(base, DirectX::XMMatrixRotationZ(arcAngle * arc));
				DirectX::XMStoreFloat3(&vertices.back().Position, v);

				vertices.emplace_back();
				v = DirectX::XMVector3Transform(base, DirectX::XMMatrixRotationZ(arcAngle * arc));
				v = DirectX::XMVectorAdd(v, offset);
				DirectX::XMStoreFloat3(&vertices.back().Position, v);
			}

			std::vector<unsigned short> indices;
			for (unsigned short arc = 0; arc < divisions; arc++)
			{
				const auto i = arc * 2;
				const auto d = divisions * 2;
				indices.push_back(i + 2);
				indices.push_back((i + 2) % d + 2);
				indices.push_back(i + 1 + 2);
				indices.push_back((i + 2) % d + 2);
				indices.push_back((i + 3) % d + 2);
				indices.push_back(i + 1 + 2);

				indices.push_back(i + 2);
				indices.push_back(centerNear);
				indices.push_back((i + 2) % d + 2);
				indices.push_back(centerFar);
				indices.push_back(i + 1 + 2);
				indices.push_back((i + 3) % d + 2);
			}

			return { std::move(vertices), std::move(indices) };
		}

		template<IsVertexElement Vertex>
		static IndexedVertices<Vertex> Create()
		{
			return CreateTesselated<Vertex>(24);
		}
	};

	class Sphere
	{
	public:
		template<IsVertexElement Vertex>
		static IndexedVertices<Vertex> CreateTesselated(int divisionPhi, int divisionTheta)
		{
			ASSERT(divisionPhi >= 3);
			ASSERT(divisionTheta >= 3);

			constexpr float radius = 1.0f;
			const auto base = DirectX::XMVectorSet(0.0f, 0.0f, radius, 0.0f);
			const float phiAngle = std::numbers::pi / divisionPhi;
			const float thetaAngle = 2.0f * std::numbers::pi / divisionTheta;

			std::vector<Vertex> vertices;
			for (int arcPhi = 1; arcPhi < divisionPhi; arcPhi++)
			{
				const auto phiBase = DirectX::XMVector3Transform(base, DirectX::XMMatrixRotationX(phiAngle * arcPhi));
				for (int arcTheta = 0; arcTheta < divisionTheta; arcTheta++)
				{
					vertices.emplace_back();
					auto v = DirectX::XMVector3Transform(phiBase, DirectX::XMMatrixRotationZ(thetaAngle * arcTheta));
					DirectX::XMStoreFloat3(&vertices.back().Position, v);
				}
			}

			const auto northPole = (unsigned short)vertices.size();
			vertices.emplace_back();
			DirectX::XMStoreFloat3(&vertices.back().Position, base);
			
			const auto southPole = (unsigned short)vertices.size();
			vertices.emplace_back();
			DirectX::XMStoreFloat3(&vertices.back().Position, DirectX::XMVectorNegate(base));

			const auto calcIndex = [divisionTheta](unsigned short arcPhi, unsigned short arcTheta)
			{
				return arcPhi * divisionTheta + arcTheta;
			};

			std::vector<unsigned short> indices;
			for (unsigned short arcPhi = 0; arcPhi < divisionPhi - 2; arcPhi++)
			{
				for (unsigned short arcTheta = 0; arcTheta < divisionTheta - 1; arcTheta++)
				{
					indices.push_back(calcIndex(arcPhi, arcTheta));
					indices.push_back(calcIndex(arcPhi + 1, arcTheta));
					indices.push_back(calcIndex(arcPhi, arcTheta + 1));
					indices.push_back(calcIndex(arcPhi, arcTheta + 1));
					indices.push_back(calcIndex(arcPhi + 1, arcTheta));
					indices.push_back(calcIndex(arcPhi + 1, arcTheta + 1));
				}
				indices.push_back(calcIndex(arcPhi, divisionTheta - 1));
				indices.push_back(calcIndex(arcPhi + 1, divisionTheta - 1));
				indices.push_back(calcIndex(arcPhi, 0));
				indices.push_back(calcIndex(arcPhi, 0));
				indices.push_back(calcIndex(arcPhi + 1, divisionTheta - 1));
				indices.push_back(calcIndex(arcPhi + 1, 0));
			}

			for (unsigned short arcTheta = 0; arcTheta < divisionTheta - 1; arcTheta++)
			{
				indices.push_back(northPole);
				indices.push_back(calcIndex(0, arcTheta));
				indices.push_back(calcIndex(0, arcTheta + 1));

				indices.push_back(calcIndex(divisionPhi - 2, arcTheta + 1));
				indices.push_back(calcIndex(divisionPhi - 2, arcTheta));
				indices.push_back(southPole);
			}

			indices.push_back(northPole);
			indices.push_back(calcIndex(0, divisionTheta - 1));
			indices.push_back(calcIndex(0, 0));

			indices.push_back(calcIndex(divisionPhi - 2, 0));
			indices.push_back(calcIndex(divisionPhi - 2, divisionTheta - 1));
			indices.push_back(southPole);

			return { std::move(vertices), std::move(indices) };
		}

		template<IsVertexElement Vertex>
		static IndexedVertices<Vertex> Create()
		{
			return CreateTesselated<Vertex>(12, 24);
		}
	};
}