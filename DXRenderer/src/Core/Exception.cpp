#include "Exception.h"
#include <sstream>
#include <iomanip>

ExceptionBase::ExceptionBase(uint32_t line, const char* file) noexcept
	: Line(line), File(file)
{
}

const char* ExceptionBase::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << " | " << GetRawMessage();
	ErrorMessage = oss.str();
	return ErrorMessage.c_str();
}

const char* ExceptionBase::GetType() const noexcept
{
	return "Exception Base";
}

std::string ExceptionBase::GetRawMessage() const noexcept
{
	std::ostringstream oss;
	oss << "File: " << std::left << std::setw(File.size() + 2) << File << std::endl <<
		"Line:" << std::setw(std::to_string(Line).length() + 1) << std::right << Line << std::endl;
	return oss.str();
}

uint32_t ExceptionBase::GetLine() const noexcept
{
	return Line;
}

const std::string& ExceptionBase::GetFile() const noexcept
{
	return File;
}


std::string ExceptionBase::TranslateErrorCode(HRESULT result) noexcept
{
	char* pMsgBuf = nullptr;
	DWORD nMsgLen = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, result, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&pMsgBuf), 0, nullptr);

	if (nMsgLen == 0)
	{
		return "Unidentified error code";
	}
	std::string errorString = pMsgBuf;
	LocalFree(pMsgBuf);
	return errorString;
}

WindowException::WindowException(uint32_t line, const char* file, HRESULT result) noexcept
	: ExceptionBase(line, file), Result(result)
{
}

const char* WindowException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() <<
		"Error Code: " << GetErrorCode() << std::endl <<
		"Error Code Demangle: " << TranslateErrorCode(Result) << std::endl <<
		GetRawMessage();
	ErrorMessage = oss.str();
	return ErrorMessage.c_str();
}

const char* WindowException::GetType() const noexcept
{
	return "Window Exception";
}

HRESULT WindowException::GetErrorCode() const noexcept
{
	return Result;
}

GraphicsException::GraphicsException(int line, const char* file, HRESULT result) noexcept
	:ExceptionBase(line, file), Result(result)
{
}

const char* GraphicsException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl <<
		"Error Code: 0x" << std::hex << std::uppercase << GetErrorCode() << std::endl <<
		std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl <<
		"Error Code Demangle: " << TranslateErrorCode(Result) << std::endl <<
		GetRawMessage();
	ErrorMessage = oss.str();
	return ErrorMessage.c_str();
}

const char* GraphicsException::GetType() const noexcept
{
	return "Graphics Exception";
}

HRESULT GraphicsException::GetErrorCode() const noexcept
{
	return Result;
}

const char* DeviceRemovedException::GetType() const noexcept
{
	return "Device Removed Exception";
}

const char* NoGraphicsException::GetType() const noexcept
{
	return "No Graphics Exception";
}
