#pragma once

#ifdef EAPI_EXPORT
#define EAPI __declspec(dllexport)
#else
#define EAPI __declspec(dllimport)
#endif

class AbstractRenderer;
extern "C" EAPI AbstractRenderer *create_blank_renderer();
extern "C" EAPI AbstractRenderer *create_vulkan_renderer();