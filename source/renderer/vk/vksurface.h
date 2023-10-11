#pragma once

class VulkanInstance;
class VulkanSurface {
public:
    VulkanSurface(VulkanInstance *instance);
    ~VulkanSurface();

    VkSurfaceKHR &get_surface() { return surface; }
    SDL_Window *get_window() { return window; }
private:
    VulkanInstance *instance = nullptr;
    SDL_Window *window = nullptr;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
};