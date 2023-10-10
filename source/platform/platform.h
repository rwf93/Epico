#pragma once

#if defined(_MSC_VER)
    #define SHARED_EXPORT __declspec(dllexport)
    #define SHARED_IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
    #define SHARED_EXPORT __attribute__((visibility("default")))
    #define SHARED_IMPORT
#else
    #define SHARED_EXPORT
    #define SHARED_IMPORT
    #error "Unsupported Compiler for shared library exporting"
#endif

#ifdef EAPI_EXPORT
    #define EAPI SHARED_EXPORT
#else
    #define EAPI SHARED_IMPORT
#endif