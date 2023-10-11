#include "vksurface.h"
#include "vkinstance.h"

VulkanSurface::VulkanSurface(VulkanInstance *instance) {
    this->instance = instance;
    window = SDL_CreateWindow("Epico", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

    if(!SDL_Vulkan_CreateSurface(window, instance->get_instance(), &surface)) {
        spdlog::info("Failed to create SDL Surface: {}", SDL_GetError());
        std::abort();
    }
};

VulkanSurface::~VulkanSurface() {
    vkb::destroy_surface(instance->get_instance(), surface);
    SDL_DestroyWindow(window);
};