#pragma once

#include <assert.h>
#include <stdint.h>
#include <memory>
#include <string>

#if _WIN32 | _WIN64
#define PLATFORM_WINDOWS
#else 
#if __linux__
#define PLATFORM_LINUX
#else
#error "unsupported platform"
#endif
#endif

#ifdef PLATFORM_WINDOWS
#define NOMINMAX
#include <Windows.h>
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(arr) sizeof(arr) / sizeof(arr[0])
#endif

#define ASSERTIONS_ENABLED

#ifdef ASSERTIONS_ENABLED

#include <iostream>
#if _MSC_VER
#define debugBreak() __debugbreak();
#else
#define debugBreak() __asm{int 3}
#endif

#define ASSERT(expr)  \
if(expr){\
}\
else{\
reportAssertionFailure(#expr, "", __FILE__, __LINE__);\
debugBreak();\
}

#define ASSERT_MSG(expr, message)\
if(expr){\
}\
else{\
reportAssertionFailure(#expr, message, __FILE__, __LINE__);\
debugBreak();\
}


#ifdef _DEBUG
#define ASSERT_DEBUG(expr) {\
if(expr){\
}\
else{\
reportAssertionFailure(#expr, "", __FILE__, __LINE__);\
debugBreak();\
}\
}
#else
#define ASSERT_DEBUG(expr)
#endif
#else
#define ASSERT(expr)
#define ASSERT_MSG(expr)
#define ASSERT_DEBUG(expr) // Does nothing
#endif

inline void reportAssertionFailure(const char* expr, const char* msg, const char* file, int line)
{
	std::cerr << "Assertion Failure: " << expr << ", message: '" << msg << "', in file: " << file << ", line: " << line << "\n";
}

inline void printMessage(std::string logType, const char* fmt, ...)
{
	printf("[%s]: ", logType.c_str());
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	printf("%c", '\n');
}

#define Debug_Warning(...) printMessage("Warning", __VA_ARGS__)
#define Debug_Error(...)   printMessage("Error", __VA_ARGS__) 
#define Debug_Log(...)     printMessage("Log", __VA_ARGS__)

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}
