#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <string>
#include <wrl.h>

struct Shader
{
	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	const Microsoft::WRL::ComPtr<ID3DBlob>& GetBlob() const;

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

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> ShaderID;
};

struct PixelShader : public Shader
{
	PixelShader(const std::string& shaderName);

	virtual void Bind() const override;

	virtual void Unbind() const override;

private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader> ShaderID;
};