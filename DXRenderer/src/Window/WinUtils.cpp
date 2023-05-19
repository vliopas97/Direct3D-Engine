#include "WinUtils.h"
#include <sstream>
#include <iomanip>

WindowClassData::WindowClassData(HINSTANCE hInstance)
	: HInstance(hInstance)
{
}

WindowClassData::operator WNDCLASSEX() const
{
	WNDCLASSEX wc{ 0 };
	wc.cbSize = CbSize;
	wc.style = Style;
	wc.lpfnWndProc = LpfnWndProc;
	wc.cbClsExtra = CbClsExtra;
	wc.cbWndExtra = CbWndExtra;
	wc.hInstance = HInstance;
	wc.hIcon = HIcon;
	wc.hCursor = HCursor;
	wc.hbrBackground = HbrBackground;
	wc.lpszMenuName = LpszMenuName;
	wc.lpszClassName = LpszClassName;
	wc.hIconSm = HIconSm;
	return wc;
}

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

std::string WindowException::TranslateErrorCode(HRESULT result) noexcept
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
