#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <fmt/core.h>
#include <fmt/format.h>

#include <VkBootstrap.h>

#include <map>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>

#include <glm/common.hpp>

#include <spdlog/spdlog.h>

#include "globals.h"
#include "vulkan.h"
