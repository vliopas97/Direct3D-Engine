#pragma once

#include "Core/Core.h"
#include <exception>
#include <string>


struct WindowClassData
{
	WindowClassData(HINSTANCE hInstance);

	operator WNDCLASSEX() const;

	UINT      CbSize = sizeof(WNDCLASSEX);
	UINT      Style = CS_OWNDC;
	WNDPROC   LpfnWndProc = DefWindowProc;
	int       CbClsExtra = 0;
	int       CbWndExtra = 0;
	HINSTANCE HInstance;
	HICON     HIcon = nullptr;
	HCURSOR   HCursor = nullptr;
	HBRUSH    HbrBackground = nullptr;
	LPCWSTR   LpszMenuName = nullptr;
	LPCWSTR   LpszClassName = L"Direct3D Engine";
	HICON     HIconSm = nullptr;
};

struct WinMessage
{
	WinMessage() = default;
	WinMessage(BOOL result, MSG msg)
		: Result(result), Message(msg)
	{
	}

	BOOL Result;
	MSG Message;

	operator int() const
	{
		return Result;
	}
};

class ExceptionBase : public std::exception
{
public:
	ExceptionBase(uint32_t line, const char* file) noexcept;
	virtual ~ExceptionBase() = default;
	const char* what() const noexcept override;
	virtual const char* GetType() const noexcept;

	std::string GetRawMessage() const noexcept;

	uint32_t GetLine() const noexcept;
	const std::string& GetFile() const noexcept;
private:
	uint32_t Line;
	std::string File;

protected:
	mutable std::string ErrorMessage;
};

#define EXCEPTION ExceptionBase(__LINE__, __FILE__)

#define EXCEPTION_WRAP(x)\
	try\
	{\
		x\
	}\
	catch( const ExceptionBase& e )\
	{\
		MessageBoxA(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION);\
	}\
	catch (const std::exception& e)\
	{\
		MessageBoxA(nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONEXCLAMATION);\
	}\
	catch (...)\
	{\
		MessageBoxA(nullptr, "No details available", "Unknown Exception", MB_OK | MB_ICONEXCLAMATION);\
	}\
	return -1;\


class WindowException : public ExceptionBase
{
public:
	WindowException(uint32_t line, const char* file, HRESULT result) noexcept;
	virtual ~WindowException() = default;
	const char* what() const noexcept override;
	virtual const char* GetType() const noexcept override;
	HRESULT GetErrorCode() const noexcept;

private:
	static std::string TranslateErrorCode(HRESULT result) noexcept;
private:
	HRESULT Result;
};

#define WIN_EXCEPTION(result) WindowException(__LINE__, __FILE__, result)
#define WIN_EXCEPTION_LAST_ERROR WindowException(__LINE__, __FILE__, GetLastError())