#include "Shader.h"

#include "CurrentGraphicsContext.h"
#include "Graphics.h"

#include <source_location>

namespace
{
	std::wstring GetCurrentPath()
	{
		std::string currentDir = std::string(std::source_location::current().file_name());
		currentDir = currentDir.substr(0, currentDir.find_last_of("\\/"));
		return std::wstring(currentDir.begin(), currentDir.end()) + L"\\Shaders\\";
	}
}

const std::wstring Shader::Path = GetCurrentPath();

const Microsoft::WRL::ComPtr<ID3DBlob>& Shader::GetBlob() const
{
		return Blob;
}

std::wstring Shader::SetUpPath(const std::string& shaderName)
{
	return Path + std::wstring(shaderName.begin(), shaderName.end()) + L".cso";
}

VertexShader::VertexShader(const std::string& shaderName)
{
	D3DReadFileToBlob(SetUpPath(shaderName).c_str(), &Blob);
	CurrentGraphicsContext::GraphicsInfo->GetDevice()->CreateVertexShader(Blob->GetBufferPointer(), Blob->GetBufferSize(), nullptr, &ShaderID);
}

void VertexShader::Bind() const
{
	CurrentGraphicsContext::GraphicsInfo->GetContext()->VSSetShader(ShaderID.Get(), nullptr, 0);
}

void VertexShader::Unbind() const
{
	CurrentGraphicsContext::GraphicsInfo->GetContext()->VSSetShader(nullptr, nullptr, 0);
}

PixelShader::PixelShader(const std::string& shaderName)
{
	D3DReadFileToBlob(SetUpPath(shaderName).c_str(), &Blob);
	CurrentGraphicsContext::GraphicsInfo->GetDevice()->CreatePixelShader(Blob->GetBufferPointer(), Blob->GetBufferSize(), nullptr, &ShaderID);
}

void PixelShader::Bind() const
{
	CurrentGraphicsContext::GraphicsInfo->GetContext()->PSSetShader(ShaderID.Get(), nullptr, 0);
}

void PixelShader::Unbind() const
{
	CurrentGraphicsContext::GraphicsInfo->GetContext()->PSSetShader(nullptr, nullptr, 0);
}
