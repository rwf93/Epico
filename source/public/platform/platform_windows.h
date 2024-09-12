#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
// Go fuck yourself Windows...
#undef max

#define SHARED_EXPORT __declspec(dllexport)
#define SHARED_IMPORT __declspec(dllimport)