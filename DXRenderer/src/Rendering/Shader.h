#pragma once

#include "Core\Core.h"

#include <array>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <string>
#include <wrl.h>

enum ShaderType
{
	VertexS = 0,
	PixelS,
	Size
};

struct Shader
{
	Shader(const std::string& shaderName);
	virtual ~Shader() = default;

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	const Microsoft::WRL::ComPtr<ID3DBlob>& GetBlob() const;
	virtual const ShaderType& GetType() const = 0;
	virtual std::string GetID() const = 0;

protected:
	std::wstring SetUpPath(const std::string& shaderName);

	virtual std::wstring GetTypeImpl() = 0;
protected:
	Microsoft::WRL::ComPtr<ID3DBlob> Blob;
	std::string Name;
private:
	static const std::wstring Path;
};

struct VertexShader : public Shader
{
	VertexShader(const std::string& shaderName);

	virtual void Bind() const override;
	virtual void Unbind() const override;
	virtual const ShaderType& GetType() const override;
	virtual std::string GetID() const override;
protected:
	std::wstring GetTypeImpl() override { return L"VS"; }

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> ShaderID;
	static const ShaderType Type = ShaderType::VertexS;
};

struct NullVertexShader : public Shader
{
	NullVertexShader();

	virtual void Bind() const override;
	virtual void Unbind() const override;

	inline  virtual const ShaderType& GetType() const override { return ShaderType::VertexS; }
	inline  virtual std::string GetID() const override { return std::string(typeid(NullVertexShader).name()); }
protected:
	std::wstring GetTypeImpl() override { return {}; }
};

struct PixelShader : public Shader
{
	PixelShader(const std::string& shaderName);

	virtual void Bind() const override;
	virtual void Unbind() const override;
	virtual const ShaderType& GetType() const override;
	virtual std::string GetID() const override;

protected:
	std::wstring GetTypeImpl() override { return L"PS"; }

private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader> ShaderID;
	static const ShaderType Type = ShaderType::PixelS;
};

struct NullPixelShader : public Shader
{
	NullPixelShader();

	virtual void Bind() const override;
	virtual void Unbind() const override;

	inline  virtual const ShaderType& GetType() const override { return ShaderType::PixelS; }
	inline  virtual std::string GetID() const override { return std::string(typeid(NullPixelShader).name()); }

protected:
	std::wstring GetTypeImpl() override { return {}; }
};

struct ShaderGroup
{
	void Add(SharedPtr<Shader> shader);

	virtual void Bind() const;
	virtual void Unbind() const;

	const Microsoft::WRL::ComPtr<ID3DBlob>& GetBlob(ShaderType type) const;
	inline size_t Size() const { return Shaders.size(); }

private:
	std::array<SharedPtr<Shader>, ShaderType::Size> Shaders;
};

class ShaderPool
{
	void Add(SharedPtr<Shader> shader);
	SharedPtr<Shader> Get(const std::string& id);

	std::unordered_map<std::string, SharedPtr<Shader>> Shaders;

	friend class Pool;
};