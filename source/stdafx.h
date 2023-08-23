#include <map>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>

#include <fmt/core.h>
#include <fmt/format.h>

#include <spdlog/spdlog.h>

#include <magic_enum.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <VkBootstrap.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <shaderc/shaderc.hpp>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>

#include <glm/glm.hpp>
