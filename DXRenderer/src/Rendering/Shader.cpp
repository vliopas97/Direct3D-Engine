#include "Shader.h"

#include "CurrentGraphicsContext.h"
#include "Graphics.h"

#include <algorithm>
#include <source_location>

namespace
{
	std::wstring GetCurrentPath()
	{
		std::string currentDir = std::string(std::source_location::current().file_name());
		currentDir = currentDir.substr(0, currentDir.find_last_of("\\/"));
		return std::wstring(currentDir.begin(), currentDir.end()) + L"\\Shaders\\build\\";
	}
}

const std::wstring Shader::Path = GetCurrentPath();

inline Shader::Shader(const std::string& shaderName)
	:Name(shaderName)
{}

const Microsoft::WRL::ComPtr<ID3DBlob>& Shader::GetBlob() const
{
		return Blob;
}

std::wstring Shader::SetUpPath(const std::string& shaderName)
{
	return Path + std::wstring(shaderName.begin(), shaderName.end()) + GetTypeImpl() + L".cso";
}

VertexShader::VertexShader(const std::string& shaderName)
	:Shader(shaderName)
{
	D3DReadFileToBlob(SetUpPath(shaderName).c_str(), &Blob);
	CurrentGraphicsContext::Device()->CreateVertexShader(Blob->GetBufferPointer(), Blob->GetBufferSize(), nullptr, &ShaderID);
}

void VertexShader::Bind() const
{
	CurrentGraphicsContext::Context()->VSSetShader(ShaderID.Get(), nullptr, 0);
}

void VertexShader::Unbind() const
{
	CurrentGraphicsContext::Context()->VSSetShader(nullptr, nullptr, 0);
}

const ShaderType& VertexShader::GetType() const
{
	return Type;
}

inline std::string VertexShader::GetID() const
{
	return std::string(typeid(VertexShader).name()) + "#" + Name;
}

PixelShader::PixelShader(const std::string& shaderName)
	:Shader(shaderName)
{
	D3DReadFileToBlob(SetUpPath(shaderName).c_str(), &Blob);
	CurrentGraphicsContext::Device()->CreatePixelShader(Blob->GetBufferPointer(), Blob->GetBufferSize(), nullptr, &ShaderID);
}

void PixelShader::Bind() const
{
	CurrentGraphicsContext::Context()->PSSetShader(ShaderID.Get(), nullptr, 0);
}

void PixelShader::Unbind() const
{
	CurrentGraphicsContext::Context()->PSSetShader(nullptr, nullptr, 0);
}

const ShaderType& PixelShader::GetType() const
{
	return Type;
}

inline std::string PixelShader::GetID() const
{
	return std::string(typeid(PixelShader).name()) + "#" + Name;
}

void ShaderGroup::Add(SharedPtr<Shader> shader)
{
	Shaders[shader->GetType()] = shader;
}

void ShaderGroup::Bind() const
{
	for (auto& shader : Shaders)
		shader->Bind();
}

void ShaderGroup::Unbind() const
{
	for (auto& shader : Shaders)
		shader->Unbind();
}

const Microsoft::WRL::ComPtr<ID3DBlob>& ShaderGroup::GetBlob(ShaderType type) const
{
	return Shaders[type]->GetBlob();
}

void ShaderPool::Add(SharedPtr<Shader> shader)
{
	if (Get(shader->GetID()))
		return;

	Shaders[shader->GetID()] = std::move(shader);
}

SharedPtr<Shader> ShaderPool::Get(const std::string& id)
{
	auto it = Shaders.find(id);
	if (it == Shaders.end())
		return {};
	else
		return it->second;
}

NullVertexShader::NullVertexShader()
	:Shader("NullVS")
{}

void NullVertexShader::Bind() const
{
	CurrentGraphicsContext::Context()->VSSetShader(nullptr, nullptr, 0);
}

void NullVertexShader::Unbind() const
{
	Bind();
}

NullPixelShader::NullPixelShader()
	:Shader("NullPS")
{}

void NullPixelShader::Bind() const
{
	CurrentGraphicsContext::Context()->VSSetShader(nullptr, nullptr, 0);
}

void NullPixelShader::Unbind() const
{
	Bind();
}
