#pragma once

#if defined(_WIN32) && !defined(__CYGWIN__)
    #include <platform/platform_win32.h>
#elif defined(__linux__)
    #include <platform/platform_linux.h>
#else
    #error "Unsupported Platform"
#endif

#ifdef EAPI_EXPORT
    #define EAPI SHARED_EXPORT
#else
    #define EAPI SHARED_IMPORT
#endif
