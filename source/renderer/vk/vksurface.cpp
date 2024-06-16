#include "vksurface.h"
#include "vkinstance.h"

VulkanSurface::VulkanSurface() {};
VulkanSurface::~VulkanSurface() {};

void VulkanSurface::init(FunctorQueue<> &queue, AppContext *app_context, VulkanInstance *vkinstance) {
    this->instance = vkinstance;
    this->context = app_context;

    context->current_window = SDL_CreateWindow(
        "Epico",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        640, 480,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN
    );

    if(!SDL_Vulkan_CreateSurface(context->current_window, instance->get_instance(), &surface)) {
        spdlog::info("Failed to create SDL Surface: {}", SDL_GetError());
        std::abort();
    }

    queue.push([&] { fini(); });
}

void VulkanSurface::fini() {
    vkb::destroy_surface(instance->get_instance(), surface);
    SDL_DestroyWindow(context->current_window);
}
