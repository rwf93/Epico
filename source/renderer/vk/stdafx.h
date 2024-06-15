#pragma once

#include <deque>
#include <functional>

#include <vulkan/vulkan_core.h>
#include <VkBootstrap.h>

#include <spdlog/spdlog.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vk_mem_alloc.h>

#define EAPI_EXPORT
#include <platform/platform.h>

#include <public/abstractrenderer.h>
#include <public/appcontext.h>
