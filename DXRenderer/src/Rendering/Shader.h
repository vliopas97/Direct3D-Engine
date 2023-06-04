#pragma once

#include "Core\Core.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <string>
#include <wrl.h>

#include <array>
#include <unordered_map>
#include <algorithm>

enum ShaderType
{
	Vertex = 0,
	Pixel,
	Size
};

struct IShader
{
	virtual ~IShader() = default;
	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;
};

struct Shader : public IShader
{
	virtual ~Shader() = default;

	const Microsoft::WRL::ComPtr<ID3DBlob>& GetBlob() const;
	virtual const ShaderType& GetType() const = 0;

protected:
	static std::wstring SetUpPath(const std::string& shaderName);

protected:
	Microsoft::WRL::ComPtr<ID3DBlob> Blob;
private:
	static const std::wstring Path;
};

struct VertexShader : public Shader
{
	VertexShader(const std::string& shaderName);

	virtual void Bind() const override;
	virtual void Unbind() const override;
	virtual const ShaderType& GetType() const override;

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> ShaderID;
	const ShaderType Type = ShaderType::Vertex;
};

struct PixelShader : public Shader
{
	PixelShader(const std::string& shaderName);

	virtual void Bind() const override;
	virtual void Unbind() const override;
	virtual const ShaderType& GetType() const override;

private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader> ShaderID;
	const ShaderType Type = ShaderType::Pixel;
};

struct ShaderGroup : public IShader
{
	std::array<UniquePtr<Shader>, ShaderType::Size> Shaders;

	void AddShader(UniquePtr<Shader> shader)
	{
		ASSERT(!Shaders[shader->GetType()]);
		Shaders[shader->GetType()] = std::move(shader);
	}

	virtual void Bind() const override
	{
		for (auto& shader : Shaders)
			shader->Bind();
	}

	virtual void Unbind() const override
	{
		for (auto& shader : Shaders)
			shader->Unbind();
	}

	const Microsoft::WRL::ComPtr<ID3DBlob>& GetBlob(ShaderType type) const
	{
		return Shaders[type]->GetBlob();
	}
};