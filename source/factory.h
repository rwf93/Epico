#pragma once

#include <platform/platform.h>

class AbstractRenderer;
extern "C" EAPI AbstractRenderer *create_blank_renderer();
extern "C" EAPI AbstractRenderer *create_vulkan_renderer();