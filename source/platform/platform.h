#pragma once

#include <string>

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
    #define WIN_LINUX(win, linux) win
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(_LINUX)
    #define PLATFORM_LINUX
    #define WIN_LINUX(win, linux) linux
#endif

#if defined(PLATFORM_WINDOWS)
    #include <platform/platform_windows.h>
#elif defined(PLATFORM_LINUX)
    #include <platform/platform_linux.h>
#else
    #error "Unsupported Platform"
#endif

#include <platform/platform_shared.h>