#pragma once

#ifdef _WIN32
	#ifdef _WIN64
		#define DF_OS_WINDOWS
		#define DF_OS_NAME "Windows x64"
	#else
		#error "Windows x86 not supported!"
	#endif
#elif defined(__linux__)
	#define DF_OS_LINUX
	#define DF_OS_NAME "LINUX"
#else
	#error "Not supported platform!"
#endif


#if defined(__clang__)
	#define DF_COMPILER_CLANG
	#define DF_COMPILER_NAME "LLVM/Clang"
#elif defined(__GNUG__)
	#define DF_COMPILER_GCC
	#define DF_COMPILER_NAME "GNU Compiler Collection"
#elif defined(_MSC_VER)
	#define DF_COMPILER_MSVC
	#define DF_COMPILER_NAME "Microsoft Visual C++"
#else
	#error Compiler not supported!
#endif

#if defined(NDEBUG)
	#define DF_RELEASE
	#define DF_MODE_NAME "Release"
#else
	#define DF_DEBUG
	#define DF_MODE_NAME "Debug"
#endif

#ifdef DF_DEBUG
	#ifdef DF_OS_WINDOWS
		#define DF_DEBUGBREAK() __debugbreak()
	#endif

	#define DF_ENABLE_ASSERT
#else
	#define DF_DEBUGBREAK()
#endif

#ifdef DF_ENABLE_ASSERT
// todo:
	//#include "Engine/Utility/Logger/Logger.h"
	#define DF_ASSERT(...) { DF_LOG_ERROR("Assertion Failed: {0}", __VA_ARGS__); DF_DEBUGBREAK(); }
	#define DF_ASSERT_IF(x, ...) { if(x) {DF_LOG_ERROR("Assertion Failed: {0}", __VA_ARGS__); DF_DEBUGBREAK(); }}
#else
	#define DF_ASSERT(...)
	#define DF_ASSERT_IF(x, ...)
#endif


#include <memory>

namespace df
{
	using ulong = uint64_t;
	using uint = uint32_t;
	using ushort = uint16_t;
	using uushort = uint8_t;

	template<typename T, typename Base>
	concept Derived = std::is_base_of_v<Base, T>;

	template<typename T, typename U>
	concept Same = std::is_same_v<T, U> && std::is_same_v<U, T>;

	template<typename T, typename... Args>
	concept Constructable = std::is_constructible_v<T, Args...>;

	template<typename Sig>
	using Fn = Sig*;


	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename... Args> requires Constructable<T, Args...>
	constexpr Scope<T> CreateScope(Args&&... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}


	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename... Args> requires Constructable<T, Args...>
	constexpr Ref<T> CreateRef(Args&&... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}

#define DF_BIND_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }s