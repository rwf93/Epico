#pragma once

#include <deque>
#include <functional>
#include <map>
#include <span>

#define VK_NO_PROTOTYPES

#include <volk.h>
#include <VkBootstrap.h>

#include <spdlog/spdlog.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vk_mem_alloc.h>

#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>

#define EAPI_EXPORT
#include <platform/platform.h>

#include <public/render/abstractrenderer.h>

#include <public/appcontext.h>

#include "vktools.h"
#include "vkinfo.h"
#include "vkconvert.h"