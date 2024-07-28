#include <deque>
#include <functional>
#include <map>
#include <span>

#define EAPI_EXPORT
#include <platform/platform.h>

// Some random library forgot to push before popping for MSVC...
#if defined(PLATFORM_WINDOWS)
    #pragma warning(push)
#endif

#define VK_NO_PROTOTYPES

#include <volk.h>
#include <VkBootstrap.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/callback_sink.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vk_mem_alloc.h>

#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>

#include <public/render/renderapi.h>

#include <public/appcontext.h>

#include "vktools.h"
#include "vkinfo.h"
#include "vkconvert.h"
