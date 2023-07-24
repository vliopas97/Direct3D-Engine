#pragma once

// target Windows 7 or later
#define _WIN32_WINNT 0x0601
#include <sdkddkver.h>

#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE

#define NOMINMAX

#include <Windows.h>
#include <memory>
#include <functional>

template<typename T>
using UniquePtr = std::unique_ptr<T>;
template<typename T, typename ... Args>
constexpr UniquePtr<T> MakeUnique(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using UniquePtrCustomDeleter = std::unique_ptr<T, std::function<void(T*)>>;

template<typename T>
using SharedPtr = std::shared_ptr<T>;
template<typename T, typename ... Args>
constexpr SharedPtr<T> MakeShared(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T>
using WeakPtr = std::weak_ptr<T>;
template<typename T, typename ... Args>
constexpr WeakPtr<T> MakeWeak(Args&& ... args)
{
	return std::weak_ptr<T>(std::forward<Args>(args)...);
}

#ifndef NDEBUG
#define ASSERT(x) { if(!(x)) { printf("Assertion Failed!"); __debugbreak(); } }
#else
#define ASSERT(x)
#endif

template < template <typename...> class base, typename derived>
struct is_base_of_template_impl
{
    template<typename... Ts>
    static constexpr std::true_type  test(const base<Ts...>*);
    static constexpr std::false_type test(...);
    using type = decltype(test(std::declval<derived*>()));
};

template < template <typename...> class base, typename derived>
using is_base_of_template = typename is_base_of_template_impl<base, derived>::type;