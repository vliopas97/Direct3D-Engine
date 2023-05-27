#pragma once
#include <string>
#include <exception>

#include "Core.h"
#include "dxerr.h"


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

protected:
	static std::string TranslateErrorCode(HRESULT result) noexcept;
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
	HRESULT Result;
};

#define WIN_EXCEPTION(result) WindowException(__LINE__, __FILE__, result)
#define WIN_EXCEPTION_LAST_ERROR WindowException(__LINE__, __FILE__, GetLastError())

class GraphicsException : public ExceptionBase
{
public:
	GraphicsException(int line, const char* file, HRESULT result) noexcept;
	const char* what() const noexcept override;
	virtual const char* GetType() const noexcept override;
	HRESULT GetErrorCode() const noexcept;
private:
	HRESULT Result;
};

class DeviceRemovedException : public GraphicsException
{
	using GraphicsException::GraphicsException;
public:
	const char* GetType() const noexcept override;
};

class NoGraphicsException : public ExceptionBase
{
public:
	using ExceptionBase::ExceptionBase;
	const char* GetType() const noexcept override;
};

#define GRAPHICS_ASSERT(hrcall) \
{ \
    HRESULT hr = (hrcall); \
    if (FAILED(hr)) \
        throw GraphicsException(__LINE__, __FILE__, hr); \
}

#define GRAPHICS_DEVICE_REMOVED_EXCEPTION(hr) DeviceRemovedException( __LINE__,__FILE__,(hr) )
#define NO_GRAPHICS_EXCEPTION() NoGraphicsException( __LINE__,__FILE__ )