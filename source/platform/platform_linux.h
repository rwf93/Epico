#pragma once

#include <dlfcn.h>

#define SHARED_EXPORT __attribute__((visibility("default")))
#define SHARED_IMPORT